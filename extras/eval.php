<!DOCTYPE html>
<pre>
<?php
   echo htmlspecialchars(shell_exec(
      'printf %s "' . addcslashes($_POST['code'], '"$`\\') . '"' . " | tr -d '\\r' | tee -a eval.log | " .
      '(ulimit -t15; MNL_PATH=mnlroot/lib/manool MNL_STACK=$((4*1024*1024)) MNL_HEAP=$((128*1024*1024)) mnlroot/bin/mnlexec /dev/stdin 2>&1)' .
      ' || printf "Exit status: $?"'));
?>
</pre>
