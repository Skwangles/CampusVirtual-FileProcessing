echo "Compiling ProcessFiles"

g++ ProcessFiles.cpp -o ProcessFiles.out

echo "Running ProcessFiles"

# add JSON directory of phone to end if you want to use the JSON files
./ProcessFiles.out /media/skwangles/disk/DCIM/Camera01/ /media/skwangles/KINGSTON/ProcessedMedia/