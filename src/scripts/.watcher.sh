f=~/.local/share/CatPi/logs.txt

du -h $f
echo ""

cat $f | grep -w Dispense | tail -n30
echo ""

cat $f | tail -n 30
