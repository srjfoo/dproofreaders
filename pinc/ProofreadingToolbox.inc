<?php
include_once($relPath.'faq.inc'); // get_faq_url()
include_once($relPath.'CharacterSelector.inc');

class ToolButton
{
    public string $html;
    public function __construct($accesskey, $title, $label, $onclick)
    {
        $access_string = $accesskey ? "accesskey='" . attr_safe($accesskey) . "'" : '';
        $this->html = "<button type='button' class='proofbutton' onclick='$onclick' $access_string title='" . attr_safe($title) . "'>$label</button>\n";
    }
}

// A link that, when clicked, opens a new window containing some tool.
class PopupLink
{
    public string $html;
    public function __construct($label, $url, $window_name, $width, $height)
    {
        $js = "return openToolboxPopup('$url', $width, $height, '$window_name');";
        $this->html = "<a href='#' class='nowrap' onClick=\"$js\"\n>$label</a>\n";
    }
}

class ProofreadingToolbox
{
    /** @var ToolButton[] */
    public array $row1_buttons;
    /** @var ToolButton[] */
    public array $row2_buttons;
    /** @var PopupLink[] */
    public array $popup_links;

    public function __construct()
    {
        $this->row1_buttons = [
            'italic' => new ToolButton('i', _('Italics'), '<i>i</i>', 'top.surroundSelection("<i>","</i>");'),
            'bold' => new ToolButton('b', _('Bold'), '<b>B</b>', 'top.surroundSelection("<b>","</b>");'),
            'small_caps' => new ToolButton('s', _('Small Caps markup'), '<span style="font-variant: small-caps;">Sc</span>', 'top.surroundSelection("<sc>","</sc>");'),
            'gesperrt' => new ToolButton('g', _('Gesperrt (spaced out)'), 'g', 'top.surroundSelection("<g>","</g>");'),
            'antiqua' => new ToolButton('f', _('Antiqua'), 'f', 'top.surroundSelection("<f>","</f>");'),
            'remove_markup' => new ToolButton('m', _('Remove markup from selection'), "<span style='text-decoration: line-through;'>&lt;x&gt;</span>", 'top.transformText("remove_markup");'),
            'upper_case' => new ToolButton('u', _('Uppercase selection'), 'ABC', 'top.transformText("upper-case");'),
            'title_case' => new ToolButton('t', _('Title case selection'), 'Abc', 'top.transformText("title-case");'),
            'lower_case' => new ToolButton('l', _('Lowercase selection'), 'abc', 'top.transformText("lower-case");'),
        ];

        $this->row2_buttons = [
            'greek' => new ToolButton('', '[Greek:', '[Greek: ]', 'top.surroundSelection("[Greek: ","]");'),
            'sidenote' => new ToolButton('', '[Sidenote:', '[Sidenote: ]', 'top.surroundSelection("[Sidenote: ","]");'),
            'illustration' => new ToolButton('', '[Illustration:', '[Illustration: ]', 'top.surroundSelection("[Illustration: ","]");'),
            'note' => new ToolButton('', _('Note'), '[** ]', 'top.surroundSelection("[** ","]");'),
            'brackets' => new ToolButton('', '[ ]', '[ ]', 'top.surroundSelection("[","]");'),
            'braces' => new ToolButton('', '{ }', '{ }', 'top.surroundSelection("{","}");'),
            'footnote' => new ToolButton('', '[Footnote #:', '[Footnote #: ]', 'top.surroundSelection("[Footnote #: ","]");'),
            'slash_star' => new ToolButton('', _('Start poetry/table'), '/* */', 'top.surroundSelection("/*\\n","\\n*/");'),
            'slash_number' => new ToolButton('', _('Start block quote'), '/# #/', 'top.surroundSelection("/#\\n","\\n#/");'),
            'thought_break' => new ToolButton('', _('Thought Break'), '&lt;tb&gt;', 'top.surroundSelection("\\n<tb>\\n","");'),
            'blank_page' => new ToolButton('', '[Blank Page]', '[Blank Page]', 'top.replaceAllText("[Blank Page]");'),
        ];

        $this->popup_links = [
            'search_and_replace' => new PopupLink(_('Search/Replace'), 'srchrep.php', 'dp_searchrepl', 330, 240),
            'table_maker' => new PopupLink(_('Table Maker'), 'mktable.php', 'dp_mktable', 600, 500),
            'greek_transliterator' => new PopupLink(_('Greek Transliterator'), 'greek2ascii.php', 'gkasciiWin', 640, 250),
        ];
    }

    public function output($projectid, $round)
    {
        echo "<div class='display-flex' id='toolbox'>";
        $character_selector = new CharacterSelector($projectid);
        echo "<div id='char-picker'>";
        echo "<button type='button' id='hide-picker' class='hide-picker-button' title='", attr_safe(_("Hide the character picker")), "'>&lang;</button>";
        $character_selector->draw();
        echo '</div>';
        echo "<button type='button' id='show-picker' class='hide-picker-button nodisp' title='", attr_safe(_("Show the character picker")), "'>&rang;</button>";

        $this->draw_tools($round);
        echo '</div>';
    }

    private function draw_tools($round)
    {
        global $code_url;

        // Which tools are shown is based on the round's pi_tools value.
        // See stages.inc where the Round()s are created for the definitive
        // list and to make changes.

        echo "<div id='tools'>";
        $elements = $this->array_extract($this->row1_buttons, $round->pi_tools['tool_buttons']);
        foreach ($elements as $element) {
            echo $element->html;
        }

        echo "<br>";
        $elements = $this->array_extract($this->row2_buttons, $round->pi_tools['tool_links']);
        foreach ($elements as $element) {
            echo $element->html;
        }

        echo "\n<span class='small'><br>\n<i>". _('Pop-up tools'). ":</i>\n";
        $elements = $this->array_extract($this->popup_links, $round->pi_tools['popup_links']);
        $popups = [];
        foreach ($elements as $element) {
            $popups[] = $element->html;
        }
        echo implode(" | ", $popups);

        echo "\n<br>\n<i>", _('Documentation'), ":</i>\n";
        $url = get_faq_url($round->document);
        echo "<a href='$url' class='nowrap' target='roundDoc'>", _('Guidelines'), "</a>";
        echo " | <a href='$code_url/faq/prooffacehelp.php' accesskey='1' class='nowrap' target='helpNewWin'>", _("Help"), "</a>";

        echo "\n</span>";
        echo "</div>";
    }

    private function array_extract($source, $keys)
    {
        if ($keys == 'ALL') {
            return $source;
        }
        return array_intersect_key($source, array_flip($keys));
    }
}
