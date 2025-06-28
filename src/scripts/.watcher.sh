f=~/.local/share/CatPi/logs.txt

du -h $f
echo ""

cat -n $f | grep -w Dispense -B1 | tail -n30
echo ""

cat -n $f | tail -n 30
