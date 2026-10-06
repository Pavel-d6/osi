#!/bin/bash
# Запуск  gcc parent.c -o parent && gcc child.c -o child
check() {
  got=$(echo "$1" | ./parent 2>&1)
  if [ "$got" == "$2" ]; then echo "OK   $1"; else echo "FAIL $1"; echo "  ожидалось: $(echo "$2" | tr '\n' ' ')"; echo "  получено:  $(echo "$got" | tr '\n' ' ')"; fi
}
check data.txt             $'6\n30\n5\n0'
check tests/empty.txt      ''
check tests/no_newline.txt '24'
check tests/negative.txt   $'-3\n-60\n100'
check tests/blank_line.txt $'2\n4'
check tests/whitespace.txt '12'
check tests/long_line.txt  $'5000\n6000'
got=$(echo "no_such_file.txt" | ./parent 2>&1)
echo "Несуществующий файл -> $got"
 
echo tests/big.txt | ./parent > /tmp/big.out 2>&1
if diff -q /tmp/big.out tests/big.expected >/dev/null; then echo "OK   tests/big.txt"; else echo "FAIL tests/big.txt"; diff /tmp/big.out tests/big.expected | head; fi
./child < tests/big.txt | diff -q - tests/big.expected >/dev/null && echo "OK   child < big.txt" || echo "FAIL child < big.txt"
 