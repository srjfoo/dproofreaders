<?php
// This file sets up code infrastructure for pages that output HTML.
// ../api/index.php handles similar infrastructure for the JSON API.
//
// This file sets up the vital infrastructure for the site, including:
// * providing exception handlers
// * (re)establishing the session
// * setting up gettext
// * enforcing maintenance mode
// * providing the require_login() function
//
// This file should be included at the top of EVERY .php page that renders
// HTML. Keeping this and its dependencies slim improves overall page
// performance.
//
// The order of the include_once()s in this file is important!

include_once($relPath."bootstrap.inc");

if (!headers_sent()) {
    header("Content-Type: text/html; charset=UTF-8");

    // Set CSP for security
    $policies = [
        // default is to just load from self
        "default-src 'self'",
        // allow inline styles
        "style-src 'self' 'unsafe-inline'",
        // allow inline scripts until we remove them, and cdn.jsdelivr.net
        // for MathJAX
        "script-src 'self' 'unsafe-inline' cdn.jsdelivr.net",
        // allow images from anywhere (due to project comments)
        "img-src 'self' *",
        // Disallow other sites from embedding pages in frames/iframes;
        // unlike other params it does not fallback to default-src
        "frame-ancestors 'self'",
    ];
    header("Content-Security-Policy: " . join("; ", $policies), false);
}

// If we don't have a database connection, don't try to resume the session.
if (DPDatabase::get_connection()) {
    dpsession_resume();
}

configure_gettext(get_desired_language());

// Load the site structure. We do this here and not in bootstrap.inc because
// it uses _() on some strings but we need to configure_gettext() first
// or those won't be in the user's language.
include_once($relPath.'site_structure.inc');

if (SiteConfig::get()->maintenance && !@$maintenance_override) {
    /*
    Including user_is.inc causes a perf impact because it includes
    page_tally.inc, which includes stages.inc - all of which have a
    great deal of overhead. That's a whole lot of work to satisfy a
    rare edge case (that we're in maintenance mode).
    Until that's resolved, conditionally include it.
    */
    include_once($relPath.'user_is.inc');
    include_once($relPath.'slim_header.inc');

    if (!user_is_a_sitemanager()) {
        $title = sprintf(_("%s Maintenance"), SiteConfig::get()->site_name);
        slim_header($title);
        echo "<h1>$title</h1>";

        $message = _("We are currently performing maintenance on the server.  Please be patient, we should have the site back up and running shortly.");
        echo "<p>$message</p>";

        if (SiteConfig::get()->maintenance_message) {
            echo SiteConfig::get()->maintenance_message;
        }

        exit();
    }
}

//----------------------------------------------------------------------------

// Exception handlers should not rely on functions outside of the base PHP
// set or defined in this file as the handlers may be used before the functions
// are defined.

function production_exception_handler($exception)
{
    global $maintenance;

    if ($exception instanceof DBConnectionError && !$maintenance) {
        // We output the timestamp in the same format and timezone (UTC) as
        // the MySQL error log (sans microseconds) for easier correlation.
        $error_message = gmdate("Y-m-d") . "T" . gmdate("H:i:s") . "Z " .  $exception->getMessage();

        // Dump the error to the php error log
        error_log("base.inc - $error_message");

        // And output a pretty message for the user. gettext isn't loaded yet
        // (because we need a connection to the DB to get the user's language)
        // so output it in English.
        echo "An error occurred connecting to the database. This is unexpected ";
        echo "as the site is not in maintenance mode. Please try again later.";
        exit(1);
    }

    echo "<p class='error'>\n";
    echo htmlspecialchars($exception->getMessage(), ENT_QUOTES, 'UTF-8');
    echo "\n</p>";
}

function test_exception_handler($exception)
{
    // Show what users would see on PROD. We don't want to call
    // production_exception_handler() here because we don't want the special
    // handling on DB connection error.
    echo "<p class='error'>\n";
    echo htmlspecialchars($exception->getMessage(), ENT_QUOTES, 'UTF-8');
    echo "\n</p>";

    // Output the stacktrace in a preformatted block
    echo "<pre>\n";
    throw $exception;
}

function require_login()
{
    global $code_url;
    global $relPath;

    // return if the user is already logged in
    if (User::is_logged_in()) {
        return;
    }

    include_once($relPath.'metarefresh.inc');

    // Remember the current REQUEST_URI, so that we can send the user there
    // after the login process.
    // (urlencode it because it might contain characters [specifically, &]
    // that would confuse parsing of the login.php URL.)
    $request_uri_encoded = urlencode($_SERVER['REQUEST_URI']);
    $url = "$code_url/accounts/require_login.php?destination=$request_uri_encoded";
    metarefresh(0, $url);
}
