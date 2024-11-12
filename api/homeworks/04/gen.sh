#!/bin/bash

UPPER_BOUND=2147483647
((UPPER_BOUND = UPPER_BOUND * 2))

generate_random_positive() {
    echo $(shuf -i 0-$UPPER_BOUND -n 1 )
}

generate_vehicles() {
    list="{"
    num_vehicles=$1
    for ((i=0; i<num_vehicles; i++)); do
        a=$(generate_random_positive)
        b=$(generate_random_positive)
        if ((a > b)); then
            tmp=$a
            a=$b
            b=$tmp
        fi
        c=$(generate_random_positive % 50)
        ((c = c % 50))
        ((c=c+1))
        d=$(generate_random_positive % 50)
        ((d = d % 50))
        ((d = d+1))
        list+="[$a-$b,$c,$d]"
        if [ "$i" -lt $((num_vehicles - 1)) ]; then
            list+=","
        fi
    done
    list+="}"
    echo "$list"
}

generate_orders() {
    num_orders=$1
    pairs=""
    for ((i=0; i<num_orders; i++)); do
        x=$(generate_random_positive)
        y=$(generate_random_positive)
        ((y = y % 50))
        pairs+="$x $y"$'\n'
    done
    echo "$pairs"
}


if [ "$2" = "small" ]; then
    UPPER_BOUND=100
fi

num_vehicles=$(($1)) 
num_orders=$(($1))
list_part=$(generate_vehicles "$num_vehicles")
pairs_part=$(generate_orders "$num_orders")

printf "%s\n%s" "$list_part" "$pairs_part"
