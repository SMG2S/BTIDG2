#!/bin/bash

rm -rf $1

git clone git@github.com:SMG2S/BTIDG2.git $1

cd $1

git checkout gh-pages

touch .nojekyll

cp -rf $2/ .

git add --all .

git commit -m "message[$((1 + $RANDOM % 100000))]: update the webpages"

git push origin gh-pages

cd ..

rm -rf $1
