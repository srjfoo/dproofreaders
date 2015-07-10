<?php
// Localization setup
include_once('languages.inc');
include_once($relPath.'site_vars.php'); // $charset $dyn_locales_dir $system_locales_dir

/* Explanation of logic:

If $_GET['lang'] is set (a page is requested as eg: default.php?lang=sr) and
requested language is installed, then use that language and set language cookie
so that after clicking on links user would remain in the language. This is
useful for external linking to a localised version.

If user is logged in, and has set language in user preferences, use that
language.

If user is not logged in, use language cookie, if there is one.

If there is no cookie try to guess an appropriate language from browser's
settings. There is no need to set the cookie because browser remains the same.

If everything else fails, default to English.

*/

if (isset($_GET['lang']) && lang_code($_GET['lang'])) {
    $intlang = lang_code($_GET['lang']);
    setcookie("language",$intlang,time()+31536000,"/");
} else if (isset($userP)  && $userP['u_intlang']){
    $intlang = $userP['u_intlang'];
} else if (!$user_is_logged_in &&
           isset($_COOKIE['language']) && $_COOKIE['language']) {
    $intlang=$_COOKIE['language'];
} else if (isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
    $langs=explode(",",$_SERVER['HTTP_ACCEPT_LANGUAGE']);
    $intlang = "";
    foreach($langs as $i) {
        $i = explode(";",$i);
        $i = trim($i[0]);
        if(lang_code($i)) {
            $intlang=lang_code($i);
            break;
        }
    }
}

// Fall back to English
if (!$intlang) {
    $intlang = "en_EN";
}

$locale = setlocale(LC_ALL, $intlang);

if(!headers_sent()) header("Content-Type: text/html; charset=$charset");

// Specify location of translation tables and choose domain
$gettext_domain = 'messages';
if ( function_exists('bindtextdomain') )
{
    bindtextdomain($gettext_domain, $dyn_locales_dir);
    bindtextdomain("iso_639", $system_locales_dir);
    if(function_exists("bind_textdomain_codeset")) {
        bind_textdomain_codeset ($gettext_domain, $charset);
        bind_textdomain_codeset ("iso_639", $charset);
    }
    textdomain($gettext_domain);
}

?>