<?php

class ProjectState
{
    public string $name;
    public string $medium_label;
    public string $label;
    public int $forum;
    public string $phase;
    public string $star_metal;

    public function __construct(
        string $name,
        string $medium_label,
        string $label,
        int $forum,
        string $phase,
        string $star_metal
    ) {
        $this->name = $name;
        $this->medium_label = $medium_label;
        $this->label = $label;
        $this->forum = $forum;
        $this->phase = $phase;
        $this->star_metal = $star_metal;
    }
}

class ProjectStates
{
    /** @var array<string, ProjectState> */
    private static array $states = [];

    public static function add_state(ProjectState $state): void
    {
        self::$states[$state->name] = $state;
    }

    /** @return string[] */
    public static function get_states(): array
    {
        return array_keys(self::$states);
    }

    /** @return array<string, ProjectState> */
    public static function get_all(): array
    {
        return self::$states;
    }

    public static function get_medium_label(?string $state): string
    {
        return self::$states[$state]->medium_label ?? '';
    }

    public static function get_label(?string $state): string
    {
        return self::$states[$state]->label ?? '';
    }

    public static function get_forum(?string $state): int
    {
        return self::$states[$state]->forum ?? -1;
    }

    public static function get_phase(?string $state): string
    {
        return self::$states[$state]->phase ?? 'NONE';
    }
}

// Note that the order in which these project states are declared
// is the order in which they will be displayed in various contexts
// (via ProjectStates::get_states()).

/**
 * @param Round[] $rounds
 * @param Pool[] $pools
 */
function declare_project_states(array $rounds, array $pools): void
{
    // for the initial creation of a project
    define("PROJ_NEW", "project_new");
    ProjectStates::add_state(new ProjectState(
        PROJ_NEW,
        _("New Project"),
        _("New Project"),
        SiteConfig::get()->waiting_projects_forum_idx,
        'NEW',
        ''
    ));

    foreach ($rounds as $round) {
        // Populate $round->project_states with all project states that apply
        // to this round for lookup later.
        $round->project_states[] = $round->project_bad_state = "{$round->id}.proj_bad";
        $round->project_states[] = $round->project_unavailable_state = "{$round->id}.proj_unavail";
        $round->project_states[] = $round->project_waiting_state = "{$round->id}.proj_waiting";
        $round->project_states[] = $round->project_available_state = "{$round->id}.proj_avail";
        $round->project_states[] = $round->project_complete_state = "{$round->id}.proj_done";

        define("PROJ_{$round->id}_BAD_PROJECT", $round->project_bad_state);
        ProjectStates::add_state(new ProjectState(
            $round->project_bad_state,
            "$round->id: " .  _("Bad Project"),
            "$round->name: " . _("Bad Project"),
            SiteConfig::get()->projects_forum_idx,
            'PAGE_EDITING',
            ''
        ));

        define("PROJ_{$round->id}_UNAVAILABLE", $round->project_unavailable_state);
        ProjectStates::add_state(new ProjectState(
            $round->project_unavailable_state,
            "$round->id: " . _("Unavailable"),
            "$round->name: " . _("Unavailable"),
            ($round->id == 'P1' ? SiteConfig::get()->waiting_projects_forum_idx : SiteConfig::get()->projects_forum_idx),
            'PAGE_EDITING',
            ''
        ));

        define("PROJ_{$round->id}_WAITING_FOR_RELEASE", $round->project_waiting_state);
        ProjectStates::add_state(new ProjectState(
            $round->project_waiting_state,
            "$round->id: " . _("Waiting"),
            "$round->name: " . _("Waiting for Release"),
            ($round->id == 'P1' ? SiteConfig::get()->waiting_projects_forum_idx : SiteConfig::get()->projects_forum_idx),
            'PAGE_EDITING',
            ''
        ));

        define("PROJ_{$round->id}_AVAILABLE", $round->project_available_state);
        ProjectStates::add_state(new ProjectState(
            $round->project_available_state,
            "$round->id: " . _("Available"),
            "$round->name: " . _("Available"),
            SiteConfig::get()->projects_forum_idx,
            'PAGE_EDITING',
            'BRONZE'
        ));

        define("PROJ_{$round->id}_COMPLETE", $round->project_complete_state);
        ProjectStates::add_state(new ProjectState(
            $round->project_complete_state,
            "$round->id: " . _("Completed"),
            "$round->name: " . _("Completed"),
            SiteConfig::get()->projects_forum_idx,
            'PAGE_EDITING',
            ''
        ));
    }

    // POST

    define("PROJ_POST_FIRST_UNAVAILABLE", "proj_post_first_unavailable");
    ProjectStates::add_state(new ProjectState(
        PROJ_POST_FIRST_UNAVAILABLE,
        _("Unavailable for PP"),
        _("Unavailable for Post-Processing"),
        SiteConfig::get()->pp_projects_forum_idx,
        'PP',
        'SILVER'
    ));

    foreach ($pools as $pool) {
        switch ($pool->id) {
            case 'PP':
                $pool->project_available_state = "proj_post_first_available";
                $pool->project_checkedout_state = "proj_post_first_checked_out";
                break;
            case 'PPV':
                $pool->project_available_state = "proj_post_second_available";
                $pool->project_checkedout_state = "proj_post_second_checked_out";
                break;
        }

        $pool->states[] = $pool->project_available_state;
        $pool->states[] = $pool->project_checkedout_state;

        define(strtoupper($pool->project_available_state), $pool->project_available_state);
        ProjectStates::add_state(new ProjectState(
            $pool->project_available_state,
            "$pool->id: " . _("Available"),
            "$pool->name: " . _("Available"),
            SiteConfig::get()->pp_projects_forum_idx,
            'PP',
            'SILVER'
        ));

        define(strtoupper($pool->project_checkedout_state), $pool->project_checkedout_state);
        ProjectStates::add_state(new ProjectState(
            $pool->project_checkedout_state,
            "$pool->id: " . _("Checked out"),
            "$pool->name: " . _("Checked out"),
            SiteConfig::get()->pp_projects_forum_idx,
            'PP',
            'SILVER'
        ));
    }

    define("PROJ_POST_COMPLETE", "proj_post_complete");
    ProjectStates::add_state(new ProjectState(
        PROJ_POST_COMPLETE,
        _("Completed Post"),
        _("Completed Post-Processing"),
        SiteConfig::get()->pp_projects_forum_idx,
        'PP',
        'SILVER'
    ));

    // SUBMIT (was GB)

    define("PROJ_SUBMIT_PG_POSTED", "proj_submit_pgposted");
    ProjectStates::add_state(new ProjectState(
        PROJ_SUBMIT_PG_POSTED,
        _("Posted to PG"),
        _("Completed and Posted to Project Gutenberg"),
        SiteConfig::get()->posted_projects_forum_idx,
        'GB',
        'GOLD'
    ));

    // for complete project

    define("PROJ_COMPLETE", "project_complete");
    ProjectStates::add_state(new ProjectState(
        PROJ_COMPLETE,
        _("Project Complete"),
        _("Project Complete"),
        SiteConfig::get()->completed_projects_forum_idx,
        'COMPLETE',
        ''
    ));

    // for the 'deletion' of a project

    define("PROJ_DELETE", "project_delete");
    ProjectStates::add_state(new ProjectState(
        PROJ_DELETE,
        _("Delete Project"),
        _("Delete Project"),
        SiteConfig::get()->deleted_projects_forum_idx,
        'NONE',
        ''
    ));

    construct_star_metal_sql();
}

/**
* construct constants for use in SQL queries:
* SQL_CONDITION_BRONZE
* SQL_CONDITION_SILVER
* SQL_CONDITION_GOLD
*/
function construct_star_metal_sql(): void
{
    foreach (ProjectStates::get_all() as $state_name => $project_state) {
        if ($project_state->star_metal) {
            $project_states_for_star_metal_[$project_state->star_metal][] = $state_name;
        }
    }
    foreach ($project_states_for_star_metal_ as $star_metal => $states) {
        $sql_constant_name = "SQL_CONDITION_$star_metal";
        $sql_condition = '(';
        foreach ($states as $project_state) {
            if ($sql_condition != '(') {
                $sql_condition .= ' OR ';
            }
            $sql_condition .= "state='$project_state'";
        }
        $sql_condition .= ')';

        define($sql_constant_name, $sql_condition);
    }
}

// -----------------------------------------------------------------------------
// these functions can be removed later
function get_medium_label_for_project_state(?string $state): string
{
    return ProjectStates::get_medium_label($state);
}

function project_states_text(?string $state): string
{
    return ProjectStates::get_label($state);
}

function get_phase_containing_project_state(?string $state): string
{
    return ProjectStates::get_phase($state);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

/**
 * Return SQL segment that can be used to sort project states
 *
 * In an SQL query, if you "ORDER BY state", it will use alphabetical order,
 * which is not very useful. Instead, ORDER BY the result of this function,
 * and it will use the canonical order-of-declaration for project states.
 */
function sql_collator_for_project_state(string $state_column): string
{
    return sprintf(
        "FIELD($state_column, %s)",
        surround_and_join(ProjectStates::get_states(), "'", "'", ",")
    );
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

/** @return string[] */
function get_project_status_descriptors(): array
{
    return ['posted', 'PPd', 'proofed', 'created'];
}

/**
 * Return an object with details about a project status
 *
 * Returned object has attributes that are useful for:
 * - finding
 * - counting or
 * - plotting a graph of
 * projects having that status.
 *
 * @param string $which
 *   A word denoting a possible status of a project, one of:
 *   `created`, `proofed`, `PPd`, or `posted`.
 *
 * @return object
 */
function get_project_status_descriptor(string $which): object
{
    $obj = new stdClass();

    switch ($which) {
        case 'created':
            $not_created_states = [PROJ_NEW, PROJ_DELETE];
            $obj->state_selector = "(state NOT IN (" . surround_and_join($not_created_states, "'", "'", ",") . "))";
            $obj->Xed_title = _('Created');
            $obj->projects_Xed_title = _('Projects Created');
            $obj->graphs_title = _('Created Projects Graphs');
            $obj->per_day_title = _('Projects Created Each Day');
            $obj->cumulative_title = _('Cumulative Projects Created');
            break;

        case 'proofed':
            $proofed_states = [
                PROJ_POST_FIRST_UNAVAILABLE,
                PROJ_POST_FIRST_AVAILABLE,
                PROJ_POST_FIRST_CHECKED_OUT,
                PROJ_POST_SECOND_AVAILABLE,
                PROJ_POST_SECOND_CHECKED_OUT,
                PROJ_SUBMIT_PG_POSTED,
                PROJ_POST_COMPLETE,
            ];
            $obj->state_selector = "(state IN (" . surround_and_join($proofed_states, "'", "'", ",") . "))";
            $obj->Xed_title = _('Proofread');
            $obj->projects_Xed_title = _('Projects Proofread');
            $obj->graphs_title = _('Proofread Projects Graphs');
            $obj->per_day_title = _('Projects Proofread Each Day');
            $obj->cumulative_title = _('Cumulative Projects Proofread');
            break;

        case 'PPd':
            $pped_states = [
                PROJ_POST_SECOND_AVAILABLE,
                PROJ_POST_SECOND_CHECKED_OUT,
                PROJ_SUBMIT_PG_POSTED,
            ];
            $obj->state_selector = "(state IN (" . surround_and_join($pped_states, "'", "'", ",") . "))";
            $obj->Xed_title = _('Post-Processed');
            $obj->projects_Xed_title = _('Projects Post-Processed');
            $obj->graphs_title = _('PPd Projects Graphs');
            $obj->per_day_title = _('Projects Post-Processed Each Day');
            $obj->cumulative_title = _('Cumulative Projects PPd');
            break;

        case 'posted':
            $obj->state_selector = "(state = '" . PROJ_SUBMIT_PG_POSTED . "')";
            $obj->Xed_title = _('Posted to PG');
            $obj->projects_Xed_title = _('Projects Posted');
            $obj->graphs_title = _('Posted Projects Graphs');
            $obj->per_day_title = _('Projects Posted to PG Each Day');
            $obj->cumulative_title = _('Cumulative Projects Posted to PG');
            break;

        default:
            throw new UnexpectedValueException(html_safe("bad value for 'which': '$which'"));
    }

    return $obj;
}
