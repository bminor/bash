recho "$*"

# If IFS is null, the parameters are joined without separators
IFS=''
recho "$*"

# If IFS is unset, the parameters are separated by spaces
unset IFS
recho "${*}"
