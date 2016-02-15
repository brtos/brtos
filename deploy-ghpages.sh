#!/bin/bash
echo -e "Publishing doxygen. \n"
HTML_PATH=build-doc
rm -rf ${HTML_PATH}
mkdir -p ${HTML_PATH}
git clone -b gh-pages "${GH_REF}" --single-branch ${HTML_PATH}
cd ${HTML_PATH}
git rm -rf .	
cp -R ../doc/html ./
git config --global user.email "travis@travis-ci.org"
git config --global user.name "travis-ci"
git add .
git commit -m "Deployed to Github Pages"
git push --force --quiet "https://${GH_TOKEN}@${GH_REF}" master:gh-pages > /dev/null 2>&1
echo -e "Published doxygen to gh-pages. \n"


