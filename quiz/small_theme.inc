<?php
include_once($relPath.'html_page_common.inc'); // output_html_header()
include_once($relPath.'theme.inc'); // headerbar_text_array()
include_once($relPath.'faq.inc');

function output_small_header($quiz)
{
    output_html_header('');

    $code_url = SiteConfig::get()->code_url;
    $quiz_name = $quiz->short_name;

    echo "\n<div id='header'>\n";
    echo "<a href='$code_url'>\n";
    echo "<div id='logo'>\n";
    echo "<img src='$code_url/graphics/dp-logo.svg' alt='" . attr_safe(SiteConfig::get()->site_name) . "'>\n";
    echo "</div>\n";
    echo "</a>";
    echo "</div>\n";

    // XXX localization bug:
    // $guide_url should point to the locale-appropriate version of the guidelines.
    if ($quiz->activity_type == "format") {
        $guide_url = get_faq_url("formatting_guidelines.php");
        $guide_text = _("Formatting Guidelines");
    } elseif ($quiz->activity_type == "proof") {
        $guide_url = get_faq_url("proofreading_guidelines.php");
        $guide_text = _("Proofreading Guidelines");
    } else {
        $guide_url = get_faq_url("faq_central.php");
        $guide_text = _("Help");
    }

    $links = [
        ['text' => SiteConfig::get()->site_abbreviation, 'url' => "$code_url/index.php", 'target' => '_top'],
        ['text' => "<span class='currentpage'>$quiz_name</span>"],
    ];

    echo "<div id='navbar-outer'>";
    echo "<div id='navbar'>";

    echo "<span id='navbar-left'>";
    echo headerbar_text_array($links);
    echo "</span>";

    $links = [
        ['text' => _("Activity Hub"), 'url' => "$code_url/activity_hub.php", 'target' => '_top'],
        ['text' => $guide_text, 'url' => $guide_url, 'target' => '_top'],
    ];

    echo "<span id='navbar-right'>";
    echo headerbar_text_array($links);
    echo "</span>";
    echo "</div>";
    echo "</div>";

    echo "<div id='content-container' style='width: 100%'>";

    // Call output_small_footer when the main page content has been emitted so
    // we can emit the footer
    register_shutdown_function('output_small_footer');
}

function output_small_footer()
{
    echo "</div>\n";
    output_html_footer();
}
