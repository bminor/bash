shcat()
{
	while read -r ; do
		echo "$REPLY"
	done
}

if [ -n "$1" ]; then
	shcat < "$1"
else
	shcat
fi
