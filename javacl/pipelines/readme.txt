
These pipelines are meant to be used from the GSPTutorial (with added libs).


You need this project + its dependencies:
 - build this project
 - do "mvn dependency:copy-dependencies"
 - go into the tuto, in lib/ folder
 - do "ln -s ~/projects/ClGaussianPyramid/javacl/target/GaussianPyramidJavaCL-*-SNAPSHOT.jar ."
 - do "ln -s ~/projects/ClGaussianPyramid/javacl/target/dependency/* ."



Then you can run the pipelines from the tutorial folder.

./run.sh ~/projects/ClGaussianPyramid/javacl/pipelines/pipeline-show-multiple-levels.xml
./run.sh ~/projects/ClGaussianPyramid/javacl/pipelines/pipeline-show-multiple-levels.xml py.useSqrt2=true l2.level=4

./run.sh ~/projects/ClGaussianPyramid/javacl/pipelines/pipeline-show-ui-level.xml
./run.sh ~/projects/ClGaussianPyramid/javacl/pipelines/pipeline-show-ui-level.xml in.uri=$(youtube-dl/youtube-dl -g iJ9wNT21c_s)

