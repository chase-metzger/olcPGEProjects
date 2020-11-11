branch_name=$(git branch --show-current)
g++ -o "bins/$branch_name"  "$branch_name/main.cpp" -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
