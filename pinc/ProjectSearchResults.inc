<?php
include_once($relPath.'Project.inc');
include_once($relPath.'ProjectTransition.inc');
include_once($relPath.'forum_interface.inc');
include_once($relPath.'wordcheck_engine.inc');
include_once($relPath.'ProjectSearchResultsConfig.inc');
include_once($relPath.'forum_interface.inc'); // get_url_to_view_topic()
include_once($relPath.'pg.inc'); // get_pg_catalog_url_for_etext
include_once($relPath.'genres.inc');  // maybe_create_temporary_genre_translation_table()

class Column
{
    public string $id;
    protected string $label;
    private string $long_label;
    private string $css_class;
    private bool $default_display;
    public bool $sortable;
    protected string $db_column;
    private Settings $userSettings;
    private string $search_origin;

    public function __construct(string $id, string $label, string $long_label, string $css_class, bool $default_display, bool $sortable, string $db_column)
    {
        $this->id = $id;
        $this->label = $label;
        $this->long_label = $long_label;
        $this->css_class = $css_class;
        $this->default_display = $default_display;
        $this->sortable = $sortable;
        $this->db_column = $db_column;
    }

    public function get_label(): string
    {
        return $this->label;
    }

    public function get_warning(): string
    {
        return '';
    }

    public function get_long_label(): string
    {
        // can also be overridden
        if (empty($this->long_label)) {
            return $this->label;
        }
        return $this->long_label;
    }

    public function get_tooltip(): string
    {
        return $this->get_long_label();
    }

    public function get_sort_sql(): ?string
    {
        return $this->db_column; // overridden by state column
    }

    public function set_datastore(Settings $userSettings, string $search_origin): void
    {
        $this->userSettings = $userSettings;
        $this->search_origin = $search_origin;
    }

    public function set_active(bool $setting): void
    {
        // We can't use set_boolean() because this actually has three states, not two:
        // * true
        // * false
        // * undefined (in which case we use the default)
        if ($setting) {
            $value = "yes";
        } else {
            $value = "no";
        }

        $this->userSettings->set_value("search:{$this->search_origin}.column:$this->id", $value);
    }

    public function is_active(): bool
    {
        // See comment in set_active() for why we can't use get/set_boolean()
        $value = $this->userSettings->get_value("search:{$this->search_origin}.column:$this->id");
        if ($value === null) {
            return $this->default_display;
        }

        return $value === 'yes';
    }

    public function echo_header_cell(string $sort_column_id, string $sql_sort_dir, string $sec_sort): void
    {
        $label = $this->get_label();
        if ($this->sortable) {
            // if we sort on a column which was previously the secondary sort
            // change secondary to the current sort column or they will be the same
            $switch_sort = [];
            if ($sort_column_id == $this->id) {
                $sec_sorter = "";
                // The header-link reverses the current sort direction.
                if ($sql_sort_dir == "asc") {
                    $dir_code = "D";
                    $marker = "&#9650;";
                } else {
                    $dir_code = "A";
                    $marker = "&#9660;";
                }
            } else { // sort ascending
                $dir_code = "A";
                $marker = "";
                if ($sec_sort == $this->id) {
                    $sec_sorter = "&nbsp;^";
                    $switch_sort = ['sec_sort' => $sort_column_id];
                } else {
                    $sec_sort_array = ['sec_sort' => $this->id, 'results_offset' => 0];
                    $sec_sorter = "&nbsp;" . make_search_page_link($sec_sort_array, "^", "#head");
                }
            }
            $pri_sort_array = array_merge(['sort' => "$this->id$dir_code", 'results_offset' => 0], $switch_sort);
            $label = make_search_page_link($pri_sort_array, $label, "#head") . "$marker$sec_sorter";
        }
        echo "<th class='$this->css_class' title='", attr_safe($this->get_tooltip()), "'>$label</th>\n";
    }

    public function echo_data_cell(ProjectSearchResultEntry $project): void
    {
        echo "<td class='$this->css_class'>";
        $this->print_cell_data($project);
        echo "</td>\n";
    }

    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        echo html_safe($project->{$this->db_column});
    }
}

class TimeColumn extends Column
{
    private string $time_format;

    public function __construct(string $id, string $label, string $long_label, string $css_class, bool $default_display, bool $sortable, string $db_column, string $time_format)
    {
        parent::__construct($id, $label, $long_label, $css_class, $default_display, $sortable, $db_column);
        $this->time_format = $time_format;
    }

    public function get_label(): string
    {
        $label = $this->label;
        if ($this->time_format == 'days') {
            // TRANSLATORS: e.g. 'Last proofed (days ago)'
            $label = sprintf(_("%s (days&nbsp;ago)"), $label);
        }
        return $label;
    }

    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        $data = $project->{$this->db_column};
        if (0 == $data) {
            echo _("never");
        } else {
            switch ($this->time_format) {
                case 'days':
                    echo sprintf("%.1f", (time() - $data) / 86400);
                    break;
                default: // case 'date':
                    echo "<span class='nowrap'>" . icu_date_template("short", $data) . "</span>";
                    break;
            }
        }
    }
}

class PGColumn extends Column
{
    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        $postednum = $project->postednum;
        echo "<a href='", get_pg_catalog_url_for_etext($postednum), "'>$postednum</a>";
    }
}

class TitleColumn extends Column
{
    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        global $code_url;
        echo "<a href='$code_url/project.php?id=$project->projectid'>" . html_safe($project->nameofwork) . "</a>";
    }
}

class LinkColumn extends Column
{
    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        $name = $project->{$this->db_column};
        if ($name != '') {
            $contact_url = attr_safe(get_url_to_compose_message_to_user($name));
            echo "<a href='$contact_url'>$name</a>";
        }
    }
}

class GenreColumn extends Column
{
    public function get_long_label(): string
    {
        return _('Difficulty') . " + " . $this->label;
    }

    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        $diff_prefix = [
            "beginner" => _("BEGINNERS"),
            "average" => "",
            "easy" => _("EASY"),
            "hard" => _("HARD"),
        ];
        $genre = $project->trans_genre;
        $prefix = $diff_prefix[$project->difficulty] ?? "";
        if ($prefix != "") {
            $genre = $prefix." ".$genre;
        }
        echo html_safe($genre);
    }
}

class StateChangeColumn extends TimeColumn
{
    public function get_warning(): string
    {
        return _("The 'Last State Change' column tracks forward progress through the proofreading and formatting rounds well, but does not reflect Post-processing state changes or state changes to Unavailable accurately.");
    }

    public function get_tooltip(): string
    {
        return _("Last State Change (see note above)");
    }
}

class StateColumn extends Column
{
    public function get_sort_sql(): ?string
    {
        return sql_collator_for_project_state('state');
    }

    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        global $pguser, $code_url;

        $transitions = get_valid_transitions($project, $pguser);
        if (count($transitions) > 0) {
            $here = attr_safe($_SERVER['REQUEST_URI']);
            echo "
                <form
                    name='$project->projectid'
                    method='POST'
                    action='$code_url/tools/changestate.php'>
                <input
                    type='hidden'
                    name='projectid'
                    value='$project->projectid'>
                <input
                    type='hidden'
                    name='curr_state'
                    value='$project->state'>
                <input
                    type='hidden'
                    name='return_uri'
                    value='$here'>
                <select
                    name='next_state'
                    onchange='this.form.submit()'>
            ";

            $this->_echo_project_state_option($project->state, true);

            foreach ($transitions as $transition) {
                $disabled = $transition->is_disabled($project);
                $optional_reason = $transition->why_disabled($project);

                $this->_echo_project_state_option($transition->to_state, false, $disabled, $optional_reason);
            }

            echo "
                </select>
                </form>
            ";
        } else {
            echo get_medium_label_for_project_state($project->state), "\n";
        }
    }

    private function _echo_project_state_option(string $project_state, bool $selected, bool $disabled = false, string $reason = ''): void
    {
        echo "<option value='" . attr_safe($project_state) . "'";
        if ($selected) {
            echo " SELECTED";
        }
        if ($disabled) {
            echo " disabled";
        }
        echo ">";
        if ($project_state == 'automodify') {
            echo _('automodify');
        } else {
            echo html_safe(get_medium_label_for_project_state($project_state));
        }
        if ($disabled) {
            echo " [$reason]";
        }
        echo "</option>\n";
    }
}

class OptionsColumn extends Column
{
    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        global $pguser, $code_url;
        $projectid = $project->projectid;
        $actions = [];

        $topic_id = $project->topic_id;
        if (!empty($topic_id)) {
            $url = get_url_to_view_topic($topic_id) . "&amp;view=unread#unread";
            $actions[] = "<a href='$url' target='_blank'>" . _("View Forum") . "</a>";
        }
        if ($project->user_can_do_quick_check()) {
            $actions[] = "<a href='$code_url/tools/project_manager/project_quick_check.php?projectid=$projectid' target='_blank'>" . _("Project Quick Check") . "</a>";
        }
        if ($project->can_be_managed_by_user($pguser)) {
            $actions[] = "<a href='$code_url/tools/project_manager/editproject.php?action=edit&amp;project=$projectid' target='_blank'>" . _("Edit Info") . "</a>";

            if ($project->dir_exists) {
                $actions[] = "<a href='$code_url/tools/project_manager/edit_project_word_lists.php?projectid=$projectid' target='_blank'>" . _("Edit Word Lists") . "</a>";
            }

            // Should we show an "attention" icon?
            // Currently, we only do this if suggestions have been added since
            // the Good Words file was last modified.
            // In future, there might be various reasons to do so.
            // (But then what would we put in the tooltip?)
            $f_g = get_project_word_file($projectid, 'good');
            $count = count_wordcheck_suggestion_events($projectid, $f_g->mod_time);
            if ($count >= 1) {
                $tooltip = attr_safe(_('"Suggestions from proofreaders" list has changed; click here to view'));
                echo "<a href='$code_url/tools/project_manager/show_good_word_suggestions.php?projectid=$projectid' target='_blank'>
                    <img src='$code_url/graphics/exclamation.gif' title='$tooltip' alt='!'></a>";
            }
        }
        if (user_can_work_in_stage($pguser, 'PP')) {
            if ($project->state == PROJ_POST_FIRST_UNAVAILABLE ||
                $project->state == PROJ_POST_FIRST_AVAILABLE ||
                $project->state == PROJ_POST_FIRST_CHECKED_OUT) {
                $actions[] = "<a href=\"$project->url/$projectid.zip\">" . _("Download") . "</a>";
            }
            if ($project->state == PROJ_POST_SECOND_CHECKED_OUT ||
                $project->state == PROJ_POST_COMPLETE) {
                $actions[] = "<a href=\"$project->url/{$projectid}_second.zip\">" . _("Download") . "</a>";
            }
        }
        if ($actions) {
            $actions = join("<br>", $actions);
            echo "<div class='dropdown'>
                <button class='dropdown-button'>", _("Links"), "</button>
                <div class='dropdown-content'>$actions</div>
                </div>";
        }
    }
}

class HoldColumn extends Column
{
    public function print_cell_data(ProjectSearchResultEntry $project): void
    {
        global $code_url;

        if ($project->hold) {
            if ($project->can_be_managed_by_current_user) {
                echo "
                    <form method='post' action='$code_url/tools/remove_project_hold.php'>
                    <input type='hidden' name='projectid' value='$project->projectid'>
                    <input type='hidden' name='curr_state' value='$project->state'>
                    <input type='hidden' name='return_uri' value='" . attr_safe($_SERVER['REQUEST_URI']) . "'>
                    <button type='submit' title='", attr_safe(_("Click to remove hold")), "'>", _("Remove hold"), "</button>
                    </form>";
            } else {
                echo "&#x2714;"; // heavy check mark
            }
        }
    }
}

class ColumnData
{
    protected Settings $userSettings;
    protected string $search_origin;
    public array $columns;

    public function __construct(Settings $userSettings, string $search_origin, string $time_format = '')
    {
        global $pguser;

        $this->userSettings = $userSettings;
        $this->search_origin = $search_origin;

        $this->columns = [
            new TitleColumn('title', _("Title"), '', 'left-align', true, true, 'nameofwork'),
            new Column("author", _('Author'), '', 'left-align', true, true, 'authorsname'),
            new Column("projectid", _('Project ID'), '', 'left-align', false, true, 'projectid'),
            new Column("language", _("Language"), '', 'left-align', false, true, 'language'),
            new GenreColumn("genre", _("Genre"), '', 'left-align', true, true, 'trans_genre'),
            new LinkColumn("project_manager", pgettext("project manager", "PM"), _('Project Manager'), 'left-align', true, true, 'username'),
            new LinkColumn("checkedoutby", _('Checked Out By'), '', 'left-align', false, true, 'checkedoutby'),
            new LinkColumn(
                "pp_er",
                // TRANSLATORS: Abbreviation for Post Processor
                _('PP'),
                _('Post-processor'),
                'left-align',
                false,
                true,
                'postproofer'
            ),
            new LinkColumn(
                "ppv_er",
                // TRANSLATORS: Abbreviation for Post-processing Verifier
                _('PPV'),
                _('Post-processing Verifier'),
                'left-align',
                false,
                true,
                'ppverifier'
            ),
            new TimeColumn("last_proof", _('Last proofed'), '', 'right-align', true, true, 't_last_page_done', $time_format),
            new PGColumn("postednum", _('PG etext number'), '', 'right-align', false, true, 'postednum'),
            new Column('p_avail', _('Available pages'), '', 'right-align', false, true, 'n_available_pages'),
            new Column('p_total', _('Total pages'), '', 'right-align', false, false, 'n_pages'),
            new StateChangeColumn("days_avail", _('Last State Change'), '', 'right-align', true, true, 'modifieddate', $time_format),
            new StateColumn('state', pgettext('project state', 'State'), '', 'left-align', true, true, 'state'),
            new HoldColumn(
                'hold',
                _('Hold'),
                // TRANSLATORS: string indicates that the project is "on hold"
                _('On hold'),
                'center-align',
                false,
                false,
                'hold'
            ),
        ];
        if (user_can_work_in_stage($pguser, 'PP') || user_is_PM()) {
            $this->columns[] = new OptionsColumn('options', _('Actions'), '', 'left-align nowrap', true, false, '');
        }

        // Set column datastore
        foreach ($this->columns as $column) {
            $column->set_datastore($this->userSettings, $this->search_origin);
        }
    }
}

class ProjectSearchResultEntry extends Project
{
    public ?string $trans_genre;
    public bool $hold;
}

class ProjectSearchResults
{
    private Settings $userSettings;
    private string $search_origin;
    private int $page_size;
    private bool $show_special_colors;
    /** @var Column[] */
    private array $active_columns;
    private string $sort_column_id;
    private ?string $sort_sql;
    private string $sql_sort_dir;
    private string $sec_sort;

    public function __construct(string $search_origin)
    {
        global $pguser;

        assert(($search_origin === 'PM') || ($search_origin === 'PS'));

        $this->userSettings = & Settings::get_Settings($pguser);
        $this->search_origin = $search_origin;
        $option_data = new OptionData($this->userSettings, $this->search_origin);
        $this->page_size = $option_data->results_per_page->get_value();
        $time_format = $option_data->time_format->get_value();

        $column_data = new ColumnData($this->userSettings, $this->search_origin, $time_format);

        // Determine whether to use special colors or not
        // (this does not affect the alternating between two
        // background colors) in the project listing.
        $this->show_special_colors = !$this->userSettings->get_boolean('hide_special_colors');

        // construct active columns array so we don't have to keep checking
        // whether the column is active (these are references)
        foreach ($column_data->columns as $column) {
            if ($column->is_active()) {
                $this->active_columns[] = $column;
            }
        }
        $this->get_sorting();
    }

    private function results_navigator(int $numrows, int $num_found_rows, int $results_offset): void
    {
        if ($results_offset > 0) {
            $prev_offset = max(0, $results_offset - $this->page_size);
            echo make_search_page_link(['results_offset' => $prev_offset], _('Previous'), "#head") . " | ";
        }
        echo sprintf(
            // TRANSLATORS: these are paging results: eg: "Projects 1 to 100 of 495"
            _("Projects %1\$d to %2\$d of %3\$d"),
            $results_offset + 1,
            $results_offset + $numrows,
            $num_found_rows
        );
        echo "\n";

        if ($results_offset + $numrows < $num_found_rows) {
            $next_offset = $results_offset + $this->page_size;
            echo " | " . make_search_page_link(['results_offset' => $next_offset], _('Next'), "#head");
        }
    }

    private function get_sort_sql(string $col_id): ?string
    {
        foreach ($this->active_columns as $column) {
            if ($column->sortable && ($column->id == $col_id)) {
                $valid = true;
                return $column->get_sort_sql();
            }
        }
        return null;
    }

    private function get_sorting(): void
    {
        // sets $this->sort_column_id, sort_sql and sql_sort_dir
        // if sort parameter is given, use it
        // else do same as last time if stored
        // else default to 'state'
        // store for next time
        $saved_sort = $this->userSettings->get_value("search:{$this->search_origin}.sort", " ");
        $sort_param = $_GET["sort"] ?? $saved_sort;

        // parse and check if valid
        // remove terminating char A or D
        $this->sort_column_id = substr($sort_param, 0, -1);
        $sort_dir = substr($sort_param, -1); // last char A or D

        $this->sort_sql = $this->get_sort_sql($this->sort_column_id);

        if ($this->sort_sql) {  // check direction D is ok else assume A
            if ($sort_dir == "D") {
                $this->sql_sort_dir = "desc";
            } else { // A or not
                $this->sql_sort_dir = "asc";
            }
        } else { // use default
            $sort_param = "stateA";
            $this->sort_column_id = 'state';
            $this->sql_sort_dir = "asc";
            $this->sort_sql = sql_collator_for_project_state('state');
        }
        if ($sort_param != $saved_sort) {
            $this->userSettings->set_value("search:{$this->search_origin}.sort", $sort_param);
        }
        $this->sort_sql .= " $this->sql_sort_dir";


        // get secondary sorting
        $saved_sec_sort = $this->userSettings->get_value("search:{$this->search_origin}.sort_sec", " ");
        $this->sec_sort = $_GET["sec_sort"] ?? $saved_sec_sort;
        $sec_sort_sql = $this->get_sort_sql($this->sec_sort);
        if (null == $sec_sort_sql) {
            $this->sec_sort = 'title';
            $sec_sort_sql = 'nameofwork';
        }
        if ($this->sec_sort != $saved_sec_sort) {
            $this->userSettings->set_value("search:{$this->search_origin}.sort_sec", $this->sec_sort);
        }

        $this->sort_sql .= ", $sec_sort_sql asc";
    }

    /** @return array{0: bool|mysqli_result, 1: int} */
    private function _search(string $where_condition, int $results_offset): array
    {
        // load a count of the total number of projects
        $sql = "
            SELECT COUNT(*)
            FROM projects
            NATURAL LEFT JOIN genre_translations
            LEFT JOIN project_holds
            USING (projectid, state)
            WHERE $where_condition
        ";
        $result = DPDatabase::query($sql);
        [$num_found_rows] = mysqli_fetch_row($result);
        $num_found_rows = (int)$num_found_rows;

        // load the result set
        $sql = "
            SELECT projects.*, genre_translations.trans_genre,
            project_holds.projectid IS NOT NULL AS hold
            FROM projects
            NATURAL LEFT JOIN genre_translations
            LEFT JOIN project_holds
            USING (projectid, state)
            WHERE $where_condition
            ORDER BY $this->sort_sql
            " . sprintf("LIMIT %d OFFSET %d", $this->page_size, $results_offset);
        if (SiteConfig::get()->testing) {
            echo_html_comment($sql);
        }
        $result = DPDatabase::query($sql);

        return [$result, $num_found_rows];
    }

    public function render(string $where_condition): void
    {
        global $pguser, $theme, $code_url;

        $results_offset = intval(@$_GET['results_offset']);

        maybe_create_temporary_genre_translation_table();
        [$result, $num_found_rows] = $this->_search($where_condition, $results_offset);

        $numrows = mysqli_num_rows($result);
        if ($numrows == 0) {
            echo "<p><b>", _("No projects matched the search criteria."), "</b></p>";
            return;
        }

        $warning = '';
        foreach ($this->active_columns as $column) {
            $warning .= $column->get_warning();
        }
        if (!empty($warning)) {
            echo "\n<p class='warning'>$warning</p>\n";
        }

        echo "<p id='head'>" . _("To set primary sort term, click on the column name; to set secondary sort term, click on the <span class='underlined'>^</span> that follows the column name.") . "</p>";
        $this->results_navigator($numrows, $num_found_rows, $results_offset);

        echo "\n<table class='themed theme_striped'>\n";
        echo "<tr>\n";
        if ($this->show_special_colors) {
            echo "<th></th>";
        }
        foreach ($this->active_columns as $column) {
            $column->echo_header_cell($this->sort_column_id, $this->sql_sort_dir, $this->sec_sort);
        }
        echo "</tr>\n";

        while ($project_assoc = mysqli_fetch_assoc($result)) {
            // TODO: rename this $project_result_entry or similar when we add type
            // defs to this file and change all of the function definitions anyway.
            $project = new ProjectSearchResultEntry($project_assoc);

            echo "<tr>\n";

            if ($this->show_special_colors) {
                $style = '';
                $cell = '';
                $special_day = get_special_day($project_assoc['special_code']);
                if (!is_null($special_day)) {
                    [$style, $cell] = get_special_day_cell_parts($special_day);
                    $style = "style='$style'";
                }
                echo "<td $style>$cell</td>";
            }

            foreach ($this->active_columns as $column) {
                $column->echo_data_cell($project);
            }
            echo "</tr>\n";
        }
        echo "</table>\n";

        $this->results_navigator($numrows, $num_found_rows, $results_offset);
        // special colours legend
        // Don't display if the user has selected the
        // setting "Show Special Colors: No".
        // The legend has been put at the bottom of the page
        // because the use of colors is presumably mostly
        // useful as a check that no typo was made. The
        // exact color probably doesn't matter and,
        // furthermore, the PM 'knows' the project and
        // what's so special about it.
        if ($this->show_special_colors) {
            echo_special_legend(" 1 = 1");
        }
    }
}

function get_search_configure_link(): string
{
    $origin = urlencode($_SERVER['REQUEST_URI']);
    return "<a href='?show=config&amp;origin=$origin'>" . _("Configure Result") . "</a>";
}

/** @param array<string, mixed> $new_array */
function make_search_page_url(array $new_array, string $anchor = ''): string
{
    return "?" . http_build_query(array_replace($_GET, $new_array)) . "$anchor";
}

/** @param array<string, mixed> $new_array */
function make_search_page_link(array $new_array, string $text, string $anchor = ''): string
{
    $url = make_search_page_url($new_array, $anchor);
    return "<a href='" . attr_safe($url) . "'>$text</a>";
}

function get_refine_search_url(): string
{
    return make_search_page_url(['show' => 'search_form']);
}

function get_refine_search_link(?string $label = null): string
{
    if (!$label) {
        $label = _("Refine Search");
    }
    $url = get_refine_search_url();
    return "<a href='" . attr_safe($url) . "'>$label</a>";
}
