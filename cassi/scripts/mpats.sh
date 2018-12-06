#!/bin/bash

rm output.xml
touch output.xml

for i in `seq 3 256`;
do
	str="s/(<imageID_>) 1 (<\/imageID_>)/\1$i\2/"
	sed -E "$str" snippet.xml >> output.xml	
	echo "" >> output.xml
done



