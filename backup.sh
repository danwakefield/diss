cd ~/documents/diss
{
git pull --rebase &>/dev/null
git push github master &>/dev/null
}
if [ $? -eq 0 ];then
    notify-send "Diss" "Backed Up"
else
    notify-send "Diss" "Backup FAILED"
fi
