<?php
$relPath = "./../../pinc/";
include_once($relPath.'base.inc');
include_once($relPath.'privacy.inc');
include_once($relPath.'theme.inc');
include_once($relPath.'page_tally.inc'); // get_page_tally_names()
include_once($relPath.'User.inc');
include_once($relPath.'graph_data.inc');
include_once('../includes/team.inc');
include_once('../includes/member.inc');

require_login();

$id = get_integer_param($_GET, 'id', null, 0, null, true);
if ($id === null) {
    $user = User::load_current();
} else {
    $user = User::load_from_uid($id);
}

$valid_tally_names = array_keys(get_page_tally_names());
$tally_name = get_enumerated_param($_GET, 'tally_name', null, $valid_tally_names, true);

$can_reveal = can_reveal_details_about($user->username, $user->u_privacy);
if ($user == User::load_current()) {
    $desc = _("Your user details");
} else {
    if ($can_reveal) {
        $user_referent = "'" . $user->username . "'";
    } else {
        $user_referent = "#" . $user->u_id;
        // Note that this doesn't reveal anything;
        // the requestor already knows the subject's u_id,
        // because it was included in the request.
    }

    $desc = sprintf(_("Details for user %s"), $user_referent);
}

output_header($desc, NO_STATSBAR, [
    "js_files" => get_graph_js_files(),
]);

echo "<h1>$desc</h1>";

if ($can_reveal) {
    if ($user->u_privacy == PRIVACY_ANONYMOUS) {
        $visibility_note = _("These stats are visible to Site Admins, Project Facilitators and the user only.");
        echo "<i>($visibility_note)</i><br>\n";
    }
    showMbrInformation($user, $tally_name);
} else {
    $brushoff = _("This user has requested that their statistics remain private.");
    echo "<p>$brushoff</p>";
}
