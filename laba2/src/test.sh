#!/bin/bash
# gcc parrent.c -o parent && gcc child.c -o child

got=$(echo data.txt | ./parent 2>&1)
if [ "$got" == $'6\n30\n5\n0' ]; then echo "OK   data.txt"; else echo "FAIL data.txt"; echo "  получено: $(echo "$got" | tr '\n' ' ')"; fi

awk 'NF { s = 0; for (i = 1; i <= NF; i++) s += $i; printf "%.0f\n", s }' tests/big.txt > /tmp/big.expected
echo tests/big.txt | ./parent > /tmp/big.out 2>&1
if diff -q /tmp/big.out /tmp/big.expected >/dev/null; then echo "OK   tests/big.txt"; else echo "FAIL tests/big.txt"; diff /tmp/big.out /tmp/big.expected | head; fi


