cd "/tmp/q56/adopt tree"
for s in D_800A3144 D_800A3258 D_800A325C D_800A3260 D_800A326C; do
  echo "== $s"
  git grep -n "\b$s\b" step14 -- src asm include | grep -v "^step14:asm/data/91C98.data.s:.*dlabel\|enddlabel\|nonmatching" | head -6
done
for o in /tmp/q56/pre04obj/*.o; do
  mipsel-linux-gnu-objdump -r "$o" | grep -E "D_800A3144|D_800A325[8C]|D_800A3260|D_800A326C" | sed "s#^#$(basename $o): #" | head -3
done
echo "word at 0x8009B0D8:"; xxd -s $((0x8009B0D8-0x80010000+0x800)) -l 8 disc/SLUS_006.63
git grep -n "D_8009B0D8\|8009B0D8" step14 -- asm/data | head -3
