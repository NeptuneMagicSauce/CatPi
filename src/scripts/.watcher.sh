f=~/.local/share/CatPi/logs/$(date +"%Y-%m-%d").txt

du -h $f
echo ""

cat $f | grep -w Dispense | tail -n30
echo ""

cat $f | tail -n 30
