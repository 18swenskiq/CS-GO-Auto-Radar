# Info 
#  Dependencies:				LINUX											WIN32 
#   GLFW3						apt-get install libglfw3-dev			?
#   Xxf86vm						apt-get install libxxf86vm-dev		N/A
#   libXi						apt-get install libxi-dev				N/A

output="./tar"
entry="main.c"
inc="-I."
lib="-L./lib -L./"
libs="-lGL -lglfw -lX11 -lXxf86vm -lXrandr -lm -lpthread -lXi -ldl"
defines=""

# debug mode
flags="-g"

while [[ "$#" -gt 0 ]]; do
	case $1 in
		-r|--release) flags="-O3"; shift ;; # release mode
		-h|--help) echo "??" ;;
		*) echo "Unkown param: $1"; exit 1 ;;
	esac
	shift
done

# generate proprocessed mess
echo "run GCC -E -C"
gcc -Wall $inc $lib -E -C $entry -o teric_building.temp.c $libs -Wl,-rpath=./ $defines
if [ $? -ne 0 ]
then
	echo "GCC preprocessor failed"
	exit 1
fi
echo "successful"

# Main build
gcc -Wall $inc $lib $flags teric_building.temp.c glad.c -o $output $libs -Wl,-rpath=./ $defines

if [ $? -ne 0 ]
then
	rm teric_building.temp.c
	echo "GCC build failed"
	exit 1
fi

rm teric_building.temp.c
echo "Build succeeded, setting up"

./tar
