<?php

ini_set('session.name', "{$site_abbreviation}_Session");
ini_set('session.use_cookies', 1);
ini_set('session.cookie_domain', ''); // A sensible value will be used.
ini_set('session.cookie_lifetime', 86400 * 31); // 31 days ~= 1 month
ini_set('session.cookie_secure', $use_secure_cookies);
ini_set('session.cookie_samesite', 'Lax');

global $pguser;
$pguser = null;

// -----------------------------------------------------------------------------

/**
 * Start a session.
 *
 * Record the session variable 'pguser' (from $userID)
 */
function dpsession_begin($userID)
{
    global $pguser;

    _session_set_handlers_and_start();

    $pguser = $userID;
    $_SESSION['pguser'] = $pguser;

    _update_user_activity_time(true);
}

/**
 * Resume a session, if one exists.
 *
 * If the request claimed to belong to a session, and that session is
 * valid/current/non-expired, then refresh the session, reinstate the
 * global variable ($pguser) of that session, and return TRUE.
 * Otherwise, return FALSE.
 */
function dpsession_resume(): void
{
    global $pguser;

    _session_set_handlers_and_start();

    if (isset($_SESSION['pguser']) && !empty($_SESSION['pguser'])) {
        // Refresh the cookie
        dp_setcookie(
            ini_get('session.name'),
            session_id(),
            time() + ini_get('session.cookie_lifetime')
        );

        // set global variable $pguser
        if (isset($_SESSION['pguser'])) {
            $pguser = $_SESSION['pguser'];
        }
        $user_is_logged_in = true;
    } else {
        session_unset();
        session_destroy();
        $user_is_logged_in = false;
    }

    if ($user_is_logged_in) {
        _update_user_activity_time(false);
    }

    // If the sender isn't logged in, and this is the first time in this
    // browsing session that they're hitting the site, we grab the request's
    // http_referer (if any), and stash that in a cookie for later use, in
    // case they register in this browsing session.
    if (!$user_is_logged_in && !isset($_COOKIE["http_referer"])) {
        if (isset($_SERVER["HTTP_REFERER"])) {
            dp_setcookie('http_referer', $_SERVER["HTTP_REFERER"]);
        } else {
            dp_setcookie('http_referer', '');
        }
    }
}

/**
 * End a session.
 */
function dpsession_end()
{
    session_unset();
    session_destroy();
}


// -----------------------------------------------------------------------------
// Internal session helper functions

function _update_user_activity_time($update_login_time_too)
{
    global $pguser;

    $settings = "t_last_activity=UNIX_TIMESTAMP()";

    // To avoid thrashing the users table by updating t_last_activity for
    // every single page load, only update t_last_activity if it hasn't
    // been updated in the last 5 minutes. This should result in reduced
    // DB load while still giving the resolution on a user's activity
    // that we want.
    $additional_where = "AND t_last_activity <= UNIX_TIMESTAMP() - 60 * 5";

    if ($update_login_time_too) {
        $settings = "$settings, last_login=UNIX_TIMESTAMP()";

        // If we're updating the login time, always run the query
        $additional_where = "";
    }

    $sql = sprintf(
        "
        UPDATE users
        SET $settings
        WHERE username='%s' $additional_where
        ",
        DPDatabase::escape($pguser)
    );
    DPDatabase::query($sql);
}

class DPSessionHandler implements SessionHandlerInterface
{
    public function open($session_path, $session_name): bool
    {
        // DB open/close is handled elsewhere, nothing to do here
        return true;
    }

    public function close(): bool
    {
        // DB open/close is handled elsewhere, nothing to do here
        return true;
    }

    #[\ReturnTypeWillChange]
    public function read($sid)
    {
        $sql = sprintf(
            "SELECT value FROM sessions WHERE sid = '%s' AND expiration > %d",
            DPDatabase::escape($sid),
            time()
        );
        $result = DPDatabase::query($sql);
        $row = mysqli_fetch_assoc($result);
        if ($row) {
            return $row["value"];
        } else {
            return "";
        }
    }

    public function write($sid, $value): bool
    {
        $expiration = time() + ini_get("session.cookie_lifetime");

        $query = sprintf(
            "
            INSERT INTO sessions
                (sid, expiration, value)
            VALUES
                ('%s', %d, '%s')
            ON DUPLICATE KEY UPDATE
                expiration = %d,
                value = '%s'
            ",
            substr(DPDatabase::escape($sid), 0, 32), // truncate to column size
            $expiration,
            DPDatabase::escape($value),
            $expiration,
            DPDatabase::escape($value)
        );
        $result = DPDatabase::query($query);
        return $result === false ? false : true;
    }

    public function destroy($sid): bool
    {
        $result = DPDatabase::query(sprintf(
            "DELETE FROM sessions WHERE sid = '%s'",
            DPDatabase::escape($sid)
        ));
        return $result === false ? false : true;
    }

    #[\ReturnTypeWillChange]
    public function gc($lifetime)
    {
        DPDatabase::query(
            sprintf(
                "DELETE FROM sessions WHERE expiration < %d",
                time() - $lifetime
            )
        );

        return DPDatabase::affected_rows();
    }
}

function _session_set_handlers_and_start()
{
    static $session_handler = null;
    if (! $session_handler) {
        $session_handler = new DPSessionHandler();
        session_set_save_handler($session_handler);
    }

    session_start();
}


// -----------------------------------------------------------------------------
// The 'debouncer' variable

function dpsession_page_set($info)
{
    $_SESSION['debouncer'] = $info;
}

function dpsession_page_is_set()
{
    return isset($_SESSION['debouncer']);
}

function dpsession_page_get()
{
    return $_SESSION['debouncer'];
}
