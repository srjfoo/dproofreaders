<?php
$relPath = '../pinc/';
include_once($relPath.'base.inc');
include_once($relPath.'send_mail.inc');
include_once($relPath.'slim_header.inc');
include_once($relPath.'links.inc');
include_once($relPath.'User.inc');

require_login();


$stage_id = @$_GET['stage_id'];
if (empty($stage_id)) {
    die("parameter 'stage_id' is empty");
}

$stage = get_Stage_for_id($stage_id);

$title = sprintf(_('Requesting access to "%s"'), $stage_id);
slim_header($title);

echo "<h2>$title</h2>\n";

$email_addr = SiteConfig::get()->promotion_requests_email_addr;

echo "<p>";
echo sprintf(_('(In case of problems, please send email to %s.)'), $email_addr);
echo "</p>";

$uao = $stage->user_access($pguser);
// echo "<pre>"; var_dump($uao); echo "</pre>";

echo "<p>";
if ($uao->can_access) {
    echo _('You already have access to this stage.');
} else {
    switch ($uao->request_status) {
        case 'sat-unneeded':
        case 'sat-granted':
        case 'unsat-granted':
            echo _("This shouldn't happen: you already have access.");
            break;

        case 'sat-requested':
        case 'unsat-requested':
            echo _('You have already requested access to this stage.');
            break;

        case 'unsat-ungranted':
            echo _('First you must satisfy the minimum requirements.');
            break;

        case 'sat-denied':
        case 'unsat-denied':
            echo _('You are not allowed to request access to this stage.');
            break;

        case 'sat-available':
            $user = User::load_current();
            if ($stage->after_satisfying_minima == 'REQ-AUTO') {
                $user->grant_access($stage_id, 'AUTO-GRANTED');
                echo _('Access has been granted!');
            } elseif ($stage->after_satisfying_minima == 'REQ-HUMAN') {
                $subject = sprintf(
                    _("Requesting Access to %1\$s--'%2\$s'"),
                    $stage_id,
                    $pguser
                );
                $body = sprintf(
                    _("User '%1\$s' has requested access to '%2\$s'"),
                    $pguser,
                    $stage_id
                );
                $body .= "\n\n";
                $body .= sprintf(
                    _("[Pending Access Requests Page](%1\$s)"),
                    "$code_url/tools/pending_access_requests.php"
                );

                send_mail($email_addr, $subject, $body);

                $user->request_access($stage_id);
                echo _('Your request has been submitted and logged.');
            } else {
                die("unexpected value for after_satisfying_minima: '$stage->after_satisfying_minima'");
            }
            break;

        default:
            die("bad request_status value: '$uao->request_status'");
    }
}
echo "</p>\n";

echo "<p>" . return_to_activity_hub_link() . "</p>\n";
