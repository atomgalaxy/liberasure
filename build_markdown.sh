#!/bin/bash

SOURCE="$1"
STYLESHEET="$2"
TARGET="$3"

if [[ ! -f "$SOURCE" ]]
then
  echo "Source not found at '$SOURCE'" &1>>2
  exit 1
fi
if [[ ! -f "$STYLESHEET" ]]
then
  echo "Stylesheet not found at '$STYLESHEET'" &1>>2
  exit 2
fi
if [[ "x$TARGET" == "x" ]]
then
  echo "Target not supplied!" &1>>2
  exit 3
fi

cat > "$TARGET" <<HD
<!DOCTYPE html>
<html lang="en">

<head>
<meta charset="utf-8">
<style type='text/css'>
HD
cat >> "$TARGET" "$STYLESHEET"
cat >> "$TARGET" <<HD
</style>
</head>
<body>
HD
# test for codehilite
python -c 'import pygments' || exit 4
python -m markdown -x codehilite "$SOURCE" >> "$TARGET" || exit 5
cat >> "$TARGET" <<HD
</body>
</html>
HD
