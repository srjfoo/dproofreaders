<?php
include_once($relPath.'prefs_options.inc'); // PRIVACY_*

/**
 * Can we reveal (to the requestor) details about the given user?
 */
function can_reveal_details_about($username, $user_privacy_setting)
{
    global $pguser; // the "requestor"

    switch ($user_privacy_setting) {
        case PRIVACY_PRIVATE:
            // Details are visible to anyone logged in.
            return !is_null($pguser);

        case PRIVACY_ANONYMOUS:
            // Details are visible to the user him/herself and to Site Admins.
            // and Project Facilitators
            return !is_null($pguser) && ($pguser == $username || user_is_proj_facilitator());

        default:
            // Shouldn't happen.
            throw new UnexpectedValueException("can_reveal_details_about(): bad privacy setting: '$user_privacy_setting'");
    }
}
