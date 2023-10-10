# the read builtin uses a consistent value of IFS to split the input line
# into fields

IFS=:
INPUT=',:abc:def,ghi'
read -r IFS var rest <<<"$INPUT"
printf 'IFS=[%s] var=[%s] rest=[%s]\n' "$IFS" "$var" "$rest"

IFS=:
INPUT=':abc:def,ghi'
read -r IFS var rest <<<"$INPUT"
printf 'IFS=[%s] var=[%s] rest=[%s]\n' "$IFS" "$var" "$rest"

IFS=:
INPUT=' :  abc:def,ghi '
read -r IFS var rest <<<"$INPUT"
printf 'IFS=[%s] var=[%s] rest=[%s]\n' "$IFS" "$var" "$rest"
