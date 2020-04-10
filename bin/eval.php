<!DOCTYPE html>
<html lang="en-US">
<meta charset="UTF-8">
<title>MANOOL Online Evaluator Output</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Inconsolata&display=swap">
<body style="margin: 0; background-color: white; color: black">
<pre style="margin: 0; font-family: Inconsolata, Consolas, monospace; font-size: 15.5px">
<?php echo htmlspecialchars(shell_exec(''
   . 'printf %s ' . escapeshellarg($_POST['code']) . " | tr -d '\\r' | tee -a eval.log | "
   . '(ulimit -t15; MNL_PATH=mnlroot/lib/manool MNL_STACK=$((4*1024*1024)) MNL_HEAP=$((128*1024*1024)) mnlroot/bin/mnlexec /dev/stdin 2>&1)'
   . ' || printf "Exit status: $?"'
), ENT_NOQUOTES, 'UTF-8')?>
</pre>
</body>
