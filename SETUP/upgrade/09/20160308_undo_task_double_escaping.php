<?php
$relPath='../../../pinc/';
include_once($relPath.'base.inc');

header('Content-type: text/plain');

// The following fields in the database have values that are stored
// double-encoded. That is, magic quotes were enabled and the values
// were encoded a second time with addslashes().
// * tasks.tasks_summary
// * tasks_task_details
// * tasks_comments.comment

// This script removes the double-encoding

// The same fields are also stored with HTML entities instead of the
// raw string. This undoes that as well.

// ------------------------------------------------------------

echo "Removing double-encoding in tasks table...\n";

$result = mysql_query("
    SELECT task_id, task_summary, task_details
    FROM tasks
") or die(mysql_error());

while ( list($task_id, $task_summary, $task_details) = mysql_fetch_row($result) )
{
    $sql = sprintf("
        UPDATE tasks
        SET task_summary = '%s', task_details = '%s'
        WHERE task_id = $task_id
    ", mysql_real_escape_string(
            htmlspecialchars_decode(stripslashes($task_summary), ENT_QUOTES)),
        mysql_real_escape_string(
            htmlspecialchars_decode(stripslashes($task_details), ENT_QUOTES))
    );

    echo "$sql\n";
    mysql_query($sql);
}

// ------------------------------------------------------------

echo "Removing double-encoding in tasks_comments table...\n";

$result = mysql_query("
    SELECT *
    FROM tasks_comments
") or die(mysql_error());

while ( list($task_id, $u_id, $comment_date, $comment) = mysql_fetch_row($result) )
{
    $sql = sprintf("
        UPDATE tasks_comments
        SET comment = '%s'
        WHERE task_id = $task_id AND u_id = $u_id AND comment_date = $comment_date
    ", mysql_real_escape_string(
            htmlspecialchars_decode(stripslashes($comment), ENT_QUOTES))
    );

    echo "$sql\n";
    mysql_query($sql);
}

// ------------------------------------------------------------

echo "\nDone!\n";

// vim: sw=4 ts=4 expandtab
