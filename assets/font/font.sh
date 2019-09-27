#!/bin/bash

function createChar() {
    font="$1"
    c="$2"
    i="$3"
    convert -background none -fill black -font "$font" -gravity center -extent 26x51 -pointsize 48 label:"$c" "chars/$i.png" > /dev/null 2>&1
}

mkdir -p chars
mkdir -p rows

function createFont() {
    font="$1"
    out="$2"

    for i in {32..126}
    do
        c=`printf "\x$(printf %x $i)"`
        createChar "$font" "$c" "$i"
    done

    createChar "$font" "\ " "32"
    createChar "$font" "\\" "92"

    col=""
    for y in {0..9}
    do
        row=""
        for x in {0..9}
        do
            r=$((10*$y))
            r=$(($r+32+$x))
            if [ -f "chars/$r.png" ]; then
                row="$row chars/$r.png"
            fi
        done

        convert +append $row "rows/row$y.png" > /dev/null
        col="$col rows/row$y.png"
    done

    convert -background none -append $col "$out" > /dev/null
}

createFont "FantasqueSansMono-Regular.otf" "font-regular.png"
createFont "FantasqueSansMono-Bold.otf" "font-bold.png"
createFont "FantasqueSansMono-Italic.otf" "font-italic.png"
