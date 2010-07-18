<?php
// Give information about a single round,
// including (most importantly) the list of projects available for work.

$relPath='../../pinc/';
include_once($relPath.'misc.inc');
include_once($relPath.'dpsql.inc');
include_once($relPath.'stages.inc');
include_once($relPath.'dp_main.inc');
include_once($relPath.'showavailablebooks.inc');
include_once($relPath.'theme.inc');
include_once($relPath.'gradual.inc');
include_once($relPath.'site_news.inc');
include_once($relPath.'mentorbanner.inc');
include_once($relPath.'page_tally.inc');

$round_id = array_get( $_GET, 'round_id', NULL );
if (is_null($round_id))
{
    echo "round.php invoked without round_id parameter.";
    exit;
}

$round = get_Round_for_round_id($round_id);
if (is_null($round_id))
{
    echo "round.php invoked with invalid round_id='$round_id'.";
    exit;
}

theme( "$round->id: $round->name", 'header' );

$uao = $round->user_access( $pguser );

$round->page_top( $uao );

// show user how to access this round
if ( !$uao->can_access )
{
    echo "<hr width='75%'>\n";
    show_user_access_object( $uao );
}

// ---------------------------------------

$pagesproofed = get_pages_proofed_maybe_simulated();

alert_re_unread_messages( $pagesproofed );

welcome_see_beginner_forum( $pagesproofed, $round->id );


show_news_for_page($round_id);


if ($pagesproofed <= 100 && $ELR_round->id == $round_id)
{
    echo "<hr width='75%'>\n";

    if ($pagesproofed > 80)
    {
        echo "<i><font size=-1>";
        printf(_("After you proofread a few more pages, the following introductory Simple Proofreading Rules will be removed from this page. However, they are permanently available <a href='%s'>here</a> if you wish to refer to them later. (You can bookmark that link if you like.)"),
            "$code_url/faq/simple_proof_rules.php");
        echo "</font></i><br><br>";
    }

    include($relPath.'simple_proof_text.inc');
}

// What guideline document are we needing?
$round_doc_url = "$code_url/faq/$round->document";

if ($pagesproofed >= 10)
{
    echo "<hr width='75%'>\n";

    if ($pagesproofed < 40)
    {
        echo "<font size=-1 face=" . $theme['font_mainbody'] . "><i>";
        printf(_("Now that you have proofread 10 pages you can see the Random Rule. Every time this page is refreshed, a random rule is selected from the <a href='%s'>Guidelines</a> and displayed in this section"),
            $round_doc_url);
        echo "<br>";
        echo _("(This explanatory line will eventually vanish.)");
        echo "</i></font><br><br>";
    }


    echo "<font face=".$theme['font_mainbody']."><b>";
    echo _("Random Rule");
    echo "</b></font><br>";


    $result = dpsql_query("SELECT anchor,subject,rule FROM rules WHERE document = '$round->document' ORDER BY RAND(NOW()) LIMIT 1");
    $rule = mysql_fetch_assoc($result);
    echo "<i>".$rule['subject']."</i><br>";
    echo "".$rule['rule']."<br>";
    // TRANSLATORS: %1$s is the linked name of a random Guideline section.
    printf(_("See the %1\$s section of the <a href='%2\$s'>Guidelines</a>"),
        "<a href='$round_doc_url#".$rule['anchor']."'>".$rule['subject']."</a>",
        $round_doc_url);
    echo "</a><br><br>";
}

thoughts_re_mentor_feedback( $pagesproofed );

if ( $round->is_a_mentor_round() )
{
    if(user_can_work_on_beginner_pages_in_round($round))
        mentor_banner($round);
}

if ($pagesproofed <= 20)
{
    if ($uao->can_access)
    {
        echo "<hr width='75%'>\n";

        echo "<font face='" . $theme['font_mainbody'] ."'><b>";
        echo _("Click on the title of a book in the list below to start proofreading.");
        echo "</b></font><br><br>\n";
    }
}
else
{
    // Link to queues.
    echo "<hr width='75%'>\n";
    echo "<h4 align='center'>", _('Release Queues'), "</h4>";
    $res = dpsql_query("
        SELECT COUNT(*)
        FROM queue_defns
        WHERE round_id='{$round->id}'
    ");
    list($n_queues) = mysql_fetch_row($res);
    if ( $n_queues == 0 )
    {
        echo sprintf(
            _("%s does not have any release queues. Any projects that enter the round's waiting area will automatically become available within a few minutes."),
            $round->id
        );
        echo "\n";
    }
    else
    {
        echo sprintf(
            _("The <a href='%s'>%s release queues</a> show the books that are waiting to become available for work in this round."),
            "$code_url/stats/release_queue.php?round_id={$round->id}",
            $round->id
        );
        echo "\n";
    }
}

// Don't display the filter block or the special colours legend to newbies.
$show_filter_block = ($pagesproofed > 20);
$allow_special_colors_legend = ($pagesproofed >= 10);

show_projects_for_round( $round, $show_filter_block, $allow_special_colors_legend );

theme('', 'footer');

// vim: sw=4 ts=4 expandtab
?>
