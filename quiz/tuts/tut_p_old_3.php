<?php
$relPath = '../../pinc/';
include_once($relPath.'base.inc');
include_once($relPath.'theme.inc');
include_once('../generic/quiz_defaults.inc'); // $blackletter_url

require_login();

output_header(_('Old Texts Proofreading Tutorial'));

echo "<h2>" . sprintf(_("Old Texts Proofreading Tutorial, page %d"), 3) . "</h2>\n";
echo "<h3>" . _("Nasal abbreviations") . "</h3>\n";
echo "<p>" . _("Sometimes an <i>n</i> or <i>m</i> is abbreviated by putting a mark over the preceding letter:") . "</p>\n";

echo "<table class='basic center-align'>\n";
echo "  <tr><th>" . _("Image") . "</th> <th>" . _("Meaning") . "</th></tr>\n";
echo "  <tr>\n";
echo "    <td><img src='../generic/images/abbr_ground.png' width='113' height='55' alt='ground'></td>\n";
echo "    <td>ground</td>\n";
echo "  </tr>\n";
echo "  <tr>\n";
echo "    <td><img src='../generic/images/abbr_France.png' width='100' height='42' alt='France'></td>\n";
echo "    <td>France</td>\n";
echo "  </tr>\n";
echo "  <tr>\n";
echo "    <td><img src='../generic/images/abbr_complete.png' width='110' height='50' alt='complete'></td>\n";
echo "    <td>complete</td>\n";
echo "  </tr>\n";
echo "</table>\n";

echo "<p>" . _("These may be proofread as macrons <kbd>ū</kbd>, tildes <kbd>ũ</kbd>, or according to the meaning (i.e. <kbd>u[n]</kbd> or <kbd>u[m]</kbd>), depending on the Project Manager's instructions.");
echo " " . _("In a regular project, if the PM does not give any guidance, please ask in the project discussion.") . "</p>\n";

echo "<p>" . sprintf(
    _("Inserting characters with diacritical marks, such as macron and tilde, is described in the guidelines: <a href='%s' target='_blank'>Characters with Diacritical Marks</a> and <a href='%s' target='_blank'>Inserting Special Characters</a>."),
    get_faq_url("proofreading_guidelines.php") . "#d_chars",
    get_faq_url("proofreading_guidelines.php") . "#insert_char"
) . "</p>\n";

echo "<h3>" . _("Blackletter") . "</h3>\n";
echo "<p>" . sprintf(
    _("Text like this: %1\$s is in a font known as <i>blackletter</i>.  If you have trouble reading it, see the wiki article on <a href='%2\$s' target='_blank'>Proofing blackletter</a> for images of the alphabet in that font."),
    "<br>\n<img src='../generic/images/blackletter_sample.png' width='207' height='52' alt='Sample Text'><br>",
    $blackletter_url
) . "</p>\n";
echo "<p>" . _("The hyphen in blackletter usually looks like a slanted equals sign.  Treat this just like a normal hyphen, and rejoin words that are hyphenated across lines.  You may need to leave hyphens as <kbd>-*</kbd> more frequently than in other projects, because spelling and hyphenation in older texts is often unpredictable.") . "</p>\n";

echo "<p><a href='../generic/main.php?quiz_page_id=p_old_3'>" . _("Continue to quiz page") . "</a></p>\n";
