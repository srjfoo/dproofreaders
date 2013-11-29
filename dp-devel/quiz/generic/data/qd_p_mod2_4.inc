<?php

function quizsolved()
{
  global $pguser;
  if (isset($pguser))
      record_quiz_attempt($pguser,"p_mod2_4","pass");
}

$browser_title                = _("Moderate Proofreading Quiz");
$welcome                      = "<h2>" . _("Moderate Proofreading Quiz, page 9") . "</h2>\n" .
                                    "<p>" . _("Try to correct the text on the bottom left, so it matches the text in the image above following the Proofreading Guidelines. When done click 'check'.") . "</p>";
$ocr_text                     = "\"It has been said,\" he began, withdrawing\nhis eyes reluctantly from an unusually large\ninsect upon the ceiling, \"that there are few\n\"situations in life that cannot be honourably\n\"settled--and without loss of time\n\"--either by suicide, a bag of gold,\n\"or by thrusting a despised anta-\n\"gonist over the edge of a precipice upon a\n\"dark night. This inoffensive person, however,\n\nA suitor\nis chosen";
$solutions                    = array("\n\"It has been said,\" he began, withdrawing\nhis eyes reluctantly from an unusually large\ninsect upon the ceiling, \"that there are few\nsituations in life that cannot be honourably\nsettled--and without loss of time--either\nby suicide, a bag of gold,\nor by thrusting a despised antagonist\nover the edge of a precipice upon a\ndark night. This inoffensive person, however,\n\nA suitor\nis chosen.");
$solved_message               = "<h2>" . _("Quiz successfully solved") . "</h2>\n" .
                                    "<p>" . _("Congratulations, no errors found!") . "</p>\n" .
                                    "<h3>" . _("Handy Fact") . "</h3>\n" .
                                    "<p>" . _("We remove extra quote marks when they appear on every line because the text will be rewrapped in post-processing, changing the line breaks.  If we left the extra quote marks in the text they would end up in the middle of the paragraph.") . "</p>";


// error messages

$messages["periodomit"] = array(
    "message_text" => "<h2>" . _("Scanno") . "</h2>\n" .
        "<p>" . _("There is still a scanno in the text: a period (<tt>.</tt>) was omitted by the OCR software.") . "</p>",
    "hints" => array(),
);
$messages["extraquote"] = array(
    "message_text" => "<h2>" . _("Quote marks on each line") . "</h2>\n" .
        "<p>" . _("Proofread these by removing all of the quote marks except for the one at the start of the quotation.") . "</p>",
    "hints" => array(),
    "guideline" => "quote_ea",
);


// error checks

$tests[] = array("type" => "expectedtext", "searchtext" =>  array("chosen."), "case_sensitive" => TRUE, "error" => "periodomit");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("however,\""), "case_sensitive" => TRUE, "error" => "P_addedclosequote");
$tests[] = array("type" => "expectedtext", "searchtext" =>  array("\n\"It"), "case_sensitive" => TRUE, "error" => "P_paratop");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("\"situations", "\"settled", "\"--", "\"by", "\"or", "\"dark", "\"over"), "case_sensitive" => TRUE, "error" => "extraquote");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array(" --", "-- "), "case_sensitive" => TRUE, "error" => "P_spacedemdash");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("---"), "case_sensitive" => TRUE, "error" => "P_em_long");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("anta-"), "case_sensitive" => TRUE, "error" => "P_hyphenleft");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("--\n"), "case_sensitive" => TRUE, "error" => "P_eoldash");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("\n--"), "case_sensitive" => TRUE, "error" => "P_soldash");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("\ntime"), "case_sensitive" => TRUE, "error" => "P_lowerlinedash");
$tests[] = array("type" => "longline", "lengthlimit" => 60, "error" => "P_longline");
$tests[] = array("type" => "wrongtextorder", "firsttext" => "person", "secondtext" => "chosen", "case_sensitive" => TRUE, "error" => "P_snmove");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("Sidenote"), "case_sensitive" => FALSE, "error" => "P_snmarkup");
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("suitor is"), "case_sensitive" => TRUE, "error" => "P_snlinebreak");
$tests[] = array("type" => "expectedtext", "searchtext" =>  array("gold,\n"), "case_sensitive" => TRUE, "error" => "P_lbchange");

// vim: sw=4 ts=4 expandtab