protoc -I=. --cpp_out=. msg.proto
protoc -I=. --cpp_out=. dbmsg.proto
#find . -name "*.h" -or -name "*.cc" | xargs sed -i "" -e "s/google::/mygame::/g"
#find . -name "*.h" -or -name "*.cc" | xargs sed -i "" -e "s/namespace\ google/namespace\ mygame/g"
