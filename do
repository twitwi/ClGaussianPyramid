
GSP=$HOME/projects/GSP
INC=../../
export LD_LIBRARY_PATH=/usr/lib:build:.

echo $LD_LIBRARY_PATH

java -Djava.library.path=$LD_LIBRARY_PATH -cp $GSP/GSPFramework/target/dependency/*:$GSP/GSPFramework/target/*:$GSP/GSPBaseutils/target/*:$INC/JavaModules/dist/JavaModules.jar fr.prima.gsp.Launcher "$@"

