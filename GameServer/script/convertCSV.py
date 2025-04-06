import csv
import json
import os
import argparse
from pathlib import Path
import sys

def parse_value(value_str: str, type_str: str, row_num: int, col_name: str):
    """
    Converts a string value from the CSV cell to the appropriate Python type
    based on the type specifier.

    Args:
        value_str: The string value from the CSV cell.
        type_str: The data type specified in the second header row (e.g., 'int', 'float', 'string', 'bool', 'json', 'array_int', 'array_string').
        row_num: The current row number (for error reporting).
        col_name: The programmatic name of the column (for error reporting).

    Returns:
        The converted value in the appropriate Python type.

    Raises:
        ValueError: If the value cannot be converted to the specified type.
        json.JSONDecodeError: If the type is 'json' and the string is invalid JSON.
    """
    type_str = type_str.lower().strip()
    # Remove leading/trailing whitespace from value *before* checking if empty
    value_str = value_str.strip()

    try:
        # Handle empty strings based on type AFTER stripping whitespace
        if not value_str:
            if type_str == 'string': return "" # Empty string remains empty string
            if type_str == 'int': return 0
            if type_str == 'float': return 0.0
            if type_str == 'bool': return False
            if type_str.startswith('array_'): return []
            if type_str == 'json': return None # Represent empty JSON cell as null
            # Default for unknown empty type
            print(f"Warning (Row {row_num}, Col '{col_name}'): Empty value for unknown type '{type_str}'. Treating as None.", file=sys.stderr)
            return None

        # Non-empty value parsing
        if type_str == 'string':
            return value_str
        elif type_str == 'int':
            return int(value_str)
        elif type_str == 'float':
            return float(value_str)
        elif type_str == 'bool':
            low_val = value_str.lower()
            if low_val in ('true', '1', 'yes', 'y'):
                return True
            elif low_val in ('false', '0', 'no', 'n'):
                return False
            else:
                # Raise error only if non-empty value is not recognized as bool
                raise ValueError(f"Invalid boolean value '{value_str}'")
        elif type_str == 'json':
            # Parse stringified JSON within the cell
             # Check if it looks like a structural type or known literal first
            if value_str.startswith(('[', '{')) or value_str.lower() in ['true', 'false', 'null']:
                return json.loads(value_str)
            else:
                # Try parsing as number if possible, else raise error
                try:
                    return json.loads(value_str) # Handles numbers directly
                except json.JSONDecodeError:
                     # If it's not a structure, literal, or number, it's invalid JSON content
                     raise ValueError(f"Invalid JSON content '{value_str}' for type 'json'. Expecting structure, number, boolean, or null.")

        elif type_str.startswith('array_'):
            # Assumes elements are separated by semicolon ';'. Handles empty elements within the list.
            element_type = type_str.split('_', 1)[1]
            # Only split if the string is not empty AFTER stripping
            elements = value_str.split(';') if value_str else []
            parsed_elements = []
            for i, elem_str in enumerate(elements):
                # Pass original row_num, add index context to col_name for errors
                context_col_name = f"{col_name}[{i}]"
                # Parse each element recursively (handles potential whitespace via strip inside)
                parsed_elements.append(parse_value(elem_str, element_type, row_num, context_col_name))
            return parsed_elements
        else:
            # Default behavior for unknown types: treat as string and print a warning
            print(f"Warning (Row {row_num}, Col '{col_name}'): Unknown type '{type_str}'. Treating value '{value_str}' as string.", file=sys.stderr)
            return value_str
    except (ValueError, TypeError) as e:
        # Catch conversion errors (e.g., int('abc'))
        raise ValueError(f"Cannot convert value '{value_str}' to type '{type_str}': {e}") from e
    except json.JSONDecodeError as e:
        # Catch errors specifically from json.loads
        raise json.JSONDecodeError(f"Invalid JSON string '{value_str}' for type 'json': {e.msg}", e.doc, e.pos) from e

def convert_csv_to_json(csv_filepath: Path, json_filepath: Path):
    """
    Converts a single CSV file with the specified 3-header structure to a JSON file,
    writing to json_filepath and overwriting if it exists.

    Returns True on success/partial success, False on critical failure preventing processing.
    """
    field_names = []
    field_types = []
    json_data = []
    line_num = 0
    had_error = False # Flag to track if non-critical errors occurred (e.g., skipping a row)

    print(f"Processing '{csv_filepath}' -> '{json_filepath}'")

    try:
        with open(csv_filepath, mode='r', encoding='utf-8-sig') as infile: # utf-8-sig handles BOM
            reader = csv.reader(infile)
            line_num += 1

            # --- Read Row 1 (Human names - skip) ---
            try:
                next(reader)
            except StopIteration:
                print(f"Error: CSV file '{csv_filepath}' is empty or has less than 2 header rows. Skipping.", file=sys.stderr)
                return False # Indicate critical failure

            line_num += 1

            # --- Read Row 2 (Programmatic names + types) ---
            try:
                header_row = next(reader)
            except StopIteration:
                print(f"Error: CSV file '{csv_filepath}' has only one header row. Skipping.", file=sys.stderr)
                return False # Indicate critical failure

            valid_headers = 0
            for i, header in enumerate(header_row):
                header = header.strip()
                if not header:
                    print(f"Warning: Empty header in column {i+1} of Row 2 in '{csv_filepath}'. Column will be skipped.", file=sys.stderr)
                    field_names.append(None) # Mark column to be skipped
                    field_types.append(None)
                    continue

                parts = header.rsplit('.', 1)
                if len(parts) == 2 and parts[0].strip() and parts[1].strip(): # Check both parts exist and aren't just whitespace
                    field_names.append(parts[0].strip())
                    field_types.append(parts[1].strip())
                    valid_headers += 1
                else:
                    print(f"Warning: Header '{header}' (Column {i+1}) in Row 2 of '{csv_filepath}' lacks valid 'name.type' format. Assuming type 'string', using full header as name.", file=sys.stderr)
                    field_names.append(header) # Use the whole header as field name
                    field_types.append('string')
                    valid_headers += 1

            if valid_headers == 0:
                 print(f"Error: No valid headers found in Row 2 of '{csv_filepath}'. Skipping file.", file=sys.stderr)
                 return False

            num_fields = len(field_names) # Total columns including skipped ones

            # --- Read Data Rows (Row 3+) ---
            for data_row in reader:
                line_num += 1
                # Skip truly empty rows (all cells empty)
                if all(not cell.strip() for cell in data_row):
                    print(f"Info: Skipping empty row {line_num} in '{csv_filepath}'.")
                    continue

                current_row_len = len(data_row)
                if current_row_len != num_fields:
                    print(f"Warning: Row {line_num} in '{csv_filepath}' has {current_row_len} columns, expected {num_fields}. Adjusting...", file=sys.stderr)
                    if current_row_len > num_fields:
                        data_row = data_row[:num_fields] # Truncate
                    else:
                        data_row.extend([''] * (num_fields - current_row_len)) # Pad

                row_obj = {}
                skip_row = False
                for i, value_str in enumerate(data_row):
                    if field_names[i] is None: # Skip columns marked due to bad header in Row 2
                        continue

                    field_name = field_names[i]
                    type_str = field_types[i]

                    try:
                        parsed_value = parse_value(value_str, type_str, line_num, field_name)
                        row_obj[field_name] = parsed_value
                    except (ValueError, json.JSONDecodeError) as e:
                        print(f"Error processing Row {line_num}, Column '{field_name}' in '{csv_filepath}': {e}. Skipping row.", file=sys.stderr)
                        skip_row = True
                        had_error = True # Mark that an error occurred in this file
                        break # Stop processing this row on first cell error

                if not skip_row:
                    json_data.append(row_obj)

        # --- Ensure output directory exists ---
        # This needs to happen *before* opening the file for writing
        try:
            json_filepath.parent.mkdir(parents=True, exist_ok=True)
        except OSError as e:
            print(f"Error: Could not create output directory '{json_filepath.parent}': {e}", file=sys.stderr)
            return False # Critical failure

        # --- Write JSON file (mode='w' handles overwrite implicitly) ---
        try:
            with open(json_filepath, mode='w', encoding='utf-8') as outfile:
                json.dump(json_data, outfile, ensure_ascii=False, indent=4) # Use indent=4 for pretty printing
        except IOError as e:
            print(f"Error: Could not write JSON file '{json_filepath}': {e}", file=sys.stderr)
            return False # Critical failure

        if not had_error:
            print(f"Successfully converted '{csv_filepath}'")
        else:
            # Output file was still written, but contained errors during processing
            print(f"Converted '{csv_filepath}' with errors (check warnings/errors above). Output written to '{json_filepath}'.")
        return True # Indicate success / partial success

    except FileNotFoundError:
        print(f"Error: File not found '{csv_filepath}'", file=sys.stderr)
        return False
    except csv.Error as e:
        print(f"CSV Error reading '{csv_filepath}' near line {line_num}: {e}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"An unexpected error occurred processing '{csv_filepath}': {e}", file=sys.stderr)
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Convert all CSV files in an input directory (using specific 3-header format) to JSON files in an output directory. Overwrites existing JSON files."
    )
    parser.add_argument(
        "input_dir",
        metavar="INPUT_DIR",
        type=str, # Read as string first for better error message if not exists
        help="Path to the input directory containing CSV files."
    )
    parser.add_argument(
        "output_dir",
        metavar="OUTPUT_DIR",
        type=str,
        help="Path to the output directory where JSON files will be saved. Will be created if it doesn't exist. Existing JSON files will be overwritten."
    )

    args = parser.parse_args()

    input_dir_path = Path(args.input_dir)
    output_dir_path = Path(args.output_dir)

    # --- Validate input directory ---
    if not input_dir_path.is_dir():
        print(f"Error: Input directory not found or is not a directory: '{input_dir_path}'", file=sys.stderr)
        sys.exit(1) # Exit script if input dir is invalid

    # --- Create base output directory (if it doesn't exist) ---
    # Do this early to catch potential permission issues
    try:
        output_dir_path.mkdir(parents=True, exist_ok=True)
        print(f"Ensured output directory exists: '{output_dir_path}'")
    except OSError as e:
         print(f"Error: Could not create output directory '{output_dir_path}': {e}", file=sys.stderr)
         sys.exit(1)


    print(f"\nSearching for CSV files recursively in: '{input_dir_path}'...")

    processed_count = 0
    success_count = 0
    # Use rglob to find CSV files recursively, case-insensitive check for suffix
    # Note: rglob itself might be case-sensitive depending on OS filesystem
    # A more robust way is to glob everything and check suffix manually if needed.
    # Let's assume case-insensitivity is handled by the filesystem or use a manual check.

    for csv_file_path in input_dir_path.rglob('*'): # Check everything first
        if csv_file_path.is_file() and csv_file_path.suffix.lower() == '.csv':
            processed_count += 1
            # Calculate relative path to preserve directory structure
            try:
                relative_path = csv_file_path.relative_to(input_dir_path)
            except ValueError:
                # Should not happen if rglob starts from input_dir_path, but handle defensively
                print(f"Warning: Could not determine relative path for '{csv_file_path}'. Skipping.", file=sys.stderr)
                continue

            # Construct output path within the output directory structure
            json_output_path = output_dir_path / relative_path.with_suffix('.json')

            # The convert function handles directory creation for the specific file
            if convert_csv_to_json(csv_file_path, json_output_path):
                # Count success only if the function didn't hit a critical early error
                success_count +=1 # Count even if there were row-level errors but file was written

    print("-" * 30)
    print(f"Processing complete.")
    print(f"Found and attempted conversion for {processed_count} CSV file(s).")
    # success_count reflects files where writing was attempted/completed, even with row errors.
    print(f"Generated/Overwritten {success_count} JSON file(s) in '{output_dir_path}'.")
    print("(Check console output above for any warnings or errors during conversion.)")


if __name__ == "__main__":
    main()