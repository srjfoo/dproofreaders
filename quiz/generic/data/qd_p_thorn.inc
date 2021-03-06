<?php


$browser_title = _("Thorn Proofreading Quiz");
$intro_title = _("Thorn Proofreading Quiz");
$initial_instructions = $initial_instructions__P;
$initial_text = "them to beware of green meat, as it weakens\nthe belly:\n\nbeware of saladis, greno metis, &amp; of frutes rawe\nfor bey make many a man liaue a feble mawe.\n]>erfore, of suche fresch lustas set not an hawe,\nFor suche wantoun appetites ai not worth a\n\nstrawe.";
$solutions = ["them to beware of green meat, as it weakens\nthe belly:\n\nbeware of saladis, grene metis, & of frutes rawe\nfor þey make many a ma[=n] haue a feble mawe.\nÞerfore, of suche fresch lustes set not an hawe,\nFor suche wantoun appetites ar not worth a\nstrawe."];


// error messages

$messages["greno"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno: an '<kbd>o</kbd>' in the text where there is an '<kbd>e</kbd>' in the image."),
];
$messages["bey"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno: a '<kbd>b</kbd>' in the text where there is a '<kbd>þ</kbd>' in the image."),
];
$messages["liaue"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno: '<kbd>li</kbd>' in the text where there is an '<kbd>h</kbd>' in the image."),
];
$messages["thornscanno"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno: '<kbd>]></kbd>' in the text where there is a '<kbd>Þ</kbd>' in the image."),
];
$messages["lustas"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno: an '<kbd>a</kbd>' in the text where there is an '<kbd>e</kbd>' in the image."),
];
$messages["ai"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno: an '<kbd>i</kbd>' in the text where there is an '<kbd>r</kbd>' in the image."),
];
$messages["macronmissing"] = [
    "message_title" => _("Scanno"),
    "message_body" => _("There is still a scanno in the text: a macron (horizontal line) over an '<kbd>n</kbd>' was omitted by the OCR software."),
];
$messages["P_macron2"] = [
    "message_title" => _("Macron"),
    "message_body" => _("Since the letter 'n' with a macron (straight line) over it does not exist as a single character in Unicode, enter it as <kbd>[=n]</kbd>."),
    "guideline" => "d_chars",
];
/*
// The quizzes use Basic Latin & Extended European Latin character suites
// which do not contain n-with-macron so don't instruct the user to use them.
$messages["macron"] = array(
    "message_title" => _("Macron"),
    "message_body" => _("Following the proofreading guidelines, please use the appropriate Unicode character for a letter with a macron (straight line) over it.  For instance, proofread an '<kbd>a</kbd>' with a macron over it as <kbd>ā</kbd>.  If the specific character does not exist in Unicode, then enter the base letter followed by the Unicode \"combining macron\" character in order to put the macron on the letter."),
    "guideline" => "d_chars",
);
$messages["macron2"] = array(
    "message_title" => _("Macron"),
    "message_body" => _("Since the letter 'n' with a macron (straight line) over it does not exist in Unicode, you should proofread it by first entering the <kbd>n</kbd> and then the combining macron character."),
    "guideline" => "d_chars",
);
*/
$messages["thorn"] = [
    "message_title" => _("Thorn incorrectly proofread"),
    "message_body" => _("The capital and lower case thorns (<kbd>Þ</kbd> and <kbd>þ</kbd>) often appear in Old and Middle English texts. These characters are available in the character picker at the bottom of the screen."),
    "wiki_ref" => sprintf(_("See the <a href='%s' target='_blank'>Thorn</a> wiki page for details."), $thorn_url),
];
$messages["thorncase"] = [
    "message_title" => _("Capital/Lower Case Thorn"),
    "message_body" => _("In the books that we proofread generally the capital thorn (<kbd>Þ</kbd>) has a much larger loop.  The lower case thorn (<kbd>þ</kbd>) has a smaller loop and often sticks down farther below the line."),
    "wiki_ref" => sprintf(_("See the <a href='%s' target='_blank'>Thorn</a> wiki page for details."), $thorn_url),
];
$messages["blankline2"] = [
    "message_title" => _("Blank Lines"),
    "message_body" => _("The OCR software incorrectly inserted a blank line within the poem.  Please remove it."),
];


// error checks

$tests[] = ["type" => "forbiddentext", "searchtext" => ["greno"], "case_sensitive" => true, "error" => "greno"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["bey"], "case_sensitive" => true, "error" => "bey"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["liaue"], "case_sensitive" => true, "error" => "liaue"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["]>"], "case_sensitive" => true, "error" => "thornscanno"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["lustas"], "case_sensitive" => true, "error" => "lustas"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["ai"], "case_sensitive" => true, "error" => "ai"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["man "], "case_sensitive" => true, "error" => "macronmissing"];
// Using diacritical markup
$tests[] = ["type" => "forbiddentext", "searchtext" => ["[n=]", "[=x]", "ñ"], "case_sensitive" => true, "error" => "P_macron"];
$tests[] = ["type" => "expectedtext", "searchtext" => ["ma[=n]"], "case_sensitive" => true, "error" => "P_macron2"];
/*
// Using combining characters
$tests[] = array("type" => "forbiddentext", "searchtext" =>  array("[=n]", "[=x]", "ñ"), "case_sensitive" => TRUE, "error" => "macron");
$tests[] = array("type" => "expectedtext", "searchtext" =>  array("man̄"), "case_sensitive" => TRUE, "error" => "macron2");
*/
$tests[] = ["type" => "forbiddentext", "searchtext" => ["pey", "per", "Der", "ber"], "case_sensitive" => false, "error" => "thorn"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["Þey", "þer"], "case_sensitive" => true, "error" => "thorncase"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["belly:\nbe"], "case_sensitive" => true, "error" => "P_poetryblanklines"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["\n\nstrawe"], "case_sensitive" => true, "error" => "blankline2"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["a strawe"], "case_sensitive" => true, "error" => "P_joinline"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["\n "], "case_sensitive" => true, "error" => "P_indent"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["/*", "*/"], "case_sensitive" => true, "error" => "P_poemmarkup"];
$tests[] = ["type" => "forbiddentext", "searchtext" => ["<i>", "</i>"], "case_sensitive" => false, "error" => "P_italmarkup"];
