set -e
doxygen Doxyfile
doxybook -i docs/doxygen/xml -o docs/doxygen -t vuepress
