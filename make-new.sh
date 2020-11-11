if [ -z "$1" ]
	then
		echo "Please supply a name for project"
		exit 1
else
	git branch $1
	git checkout $1
	mkdir $1
	cp main.cpp $1/main.cpp
fi
