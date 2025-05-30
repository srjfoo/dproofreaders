<?php

include_once($relPath.'dpsql.inc');
include_once($relPath.'page_tally.inc');

function get_graph_js_files()
{
    global $code_url;
    return [
        "$code_url/node_modules/d3/dist/d3.min.js",
        "$code_url/scripts/svgGraphs.js",
    ];
}

function build_svg_graph_inits($graphs)
{
    foreach ($graphs as [$type, $id, $config]) {
        $js_data[] = "$type('$id', " . json_encode($config) . ");";
    }
    return "window.addEventListener('DOMContentLoaded', function(){" . join($js_data) . "});";
}

function cumulative_total_proj_summary_graph()
{
    $max_num_data = 0;
    $data = [];

    foreach (get_project_status_descriptors() as $which) {
        $psd = get_project_status_descriptor($which);

        //query db and put results into arrays
        $sql = sprintf(
            "
            SELECT date, SUM(num_projects)
            FROM project_state_stats
            WHERE %s
            GROUP BY DATE
            ORDER BY date ASC
            ",
            $psd->state_selector
        );
        $result = DPDatabase::query($sql);

        [$datax, $datay] = dpsql_fetch_columns($result);

        if (empty($datay)) {
            $datay[0] = 0;
        }

        if (count($datay) > $max_num_data) {
            $max_num_data = count($datay);
        }

        $data[$which] = ["x" => $datax, "y" => $datay];
    }

    return [
        "data" => $data,
        "title" => _("Total Projects Created, Proofread, Post-Processed and Posted"),
        "downloadLabel" => _("Download Chart"),
    ];
}

function user_logging_on($past, $preceding)
{
    // For each hour in the $past interval,
    // show the number of (distinct) users who had logged in
    // (at least once) during the $preceding interval.

    $seconds_per_day = 24 * 60 * 60;

    switch ($past) {
        case 'year':
            $min_timestamp = time() - 366 * $seconds_per_day;
            $date_format = '%Y-%m-%d';
            break;

        case 'day':
            $min_timestamp = time() - $seconds_per_day;
            $date_format = '%H';
            break;

        default:
            throw new UnexpectedValueException("bad value for 'past'");
    }

    switch ($preceding) {
        case 'hour':
            $title = _("Number of users newly logged in each hour");
            $column_name = 'L_hour';
            break;

        case 'day':
            $title = _('Number of users newly logged in over 24 hours');
            $column_name = 'L_day';
            break;

        case 'week':
            $title = _("Number of users newly logged in over 7 days");
            $column_name = 'L_week';
            break;

        case 'fourweek':
            $title = _("Number of users newly logged in over 28 days");
            $column_name = 'L_4wks';
            break;

        default:
            throw new UnexpectedValueException("bad value for 'preceding'");
    }

    ///////////////////////////////////////////////////
    //query db and put results into arrays

    $sql = sprintf(
        "
        SELECT DATE_FORMAT(FROM_UNIXTIME(time_stamp), '%s'), $column_name
        FROM user_active_log
        WHERE time_stamp >= %d
        ORDER BY time_stamp
        ",
        DPDatabase::escape($date_format),
        $min_timestamp
    );
    $result = DPDatabase::query($sql);

    [$datax, $datay] = dpsql_fetch_columns($result);

    return [
        "title" => $title,
        "downloadLabel" => _("Download Chart"),
        "axisLeft" => true,
        "data" => [
            _('Fresh Logons') => [
                "x" => $datax,
                "y" => $datay,
            ],
        ],
    ];
}

function tallyboard_deltas($tally_name, $holder_type, $holder_id, $days_back)
{
    $valid_tally_names = array_keys(get_page_tally_names());

    $pages_per_day = get_pages_per_day_for_past_n_days($tally_name, $holder_type, $holder_id, $days_back);

    $datax = array_keys($pages_per_day);
    $datay = array_values($pages_per_day);

    $error = null;
    if (empty($datax) || empty($datay)) {
        if ($holder_type == 'U') {
            $error = _("This user has not completed any pages in this round.");
        } else {
            $error = _("This team has not completed any pages in this round.");
        }
    }

    return [
        "title" => _('Pages Completed per Day'),
        "downloadLabel" => _("Download Chart"),
        "bottomLegend" => $error,
        "width" => 600,
        "height" => 300,
        "xAxisHeight" => 65,
        "data" => [
            is_null($error) ? _('Pages') : $error => [
                "x" => $datax,
                "y" => $datay,
            ],
        ],
    ];
}

function new_users($time_interval)
{
    $time_interval_options = [
        'day' => [
            'format' => '%Y-%b-%d',
            'title' => _("Number of users registered per day"),
        ],
        'week' => [
            'format' => '%Y-%U',
            'title' => _("Number of users registered per week"),
        ],
        'month' => [
            'format' => '%Y-%M',
            'title' => _("Number of users registered per month"),
        ],
        'year' => [
            'format' => '%Y',
            'title' => _("Number of users registered per year"),
        ],
    ];

    $date_format = $time_interval_options[$time_interval]['format'];
    $title = $time_interval_options[$time_interval]['title'];

    $sql = sprintf(
        "
        SELECT FROM_UNIXTIME(date_created, '%s'), COUNT(*)
        FROM users
        GROUP BY 1
        ORDER BY date_created
        ",
        DPDatabase::escape($date_format)
    );
    $res = DPDatabase::query($sql);

    [$datax, $datay] = dpsql_fetch_columns($res);

    return [
        "title" => $title,
        "downloadLabel" => _("Download Chart"),
        "xAxisHeight" => 90,
        "data" => [
            _('# users') => [
                "x" => $datax,
                "y" => $datay,
            ],
        ],
    ];
}

function average_hour_users_logging_on()
{
    ///////////////////////////////////////////////////
    //Numbers of users logging on in each hour of the day, since the start of stats

    //query db and put results into arrays
    $sql = "
        SELECT hour, AVG(L_hour)
        FROM user_active_log
        GROUP BY hour
        ORDER BY hour
    ";
    $result = DPDatabase::query($sql);

    [$datax, $datay] = dpsql_fetch_columns($result);

    return [
        "title" => _('Average number of users newly logged in each hour'),
        "downloadLabel" => _("Download Chart"),
        "data" => [
            _('Fresh Logons') => [
                "x" => $datax,
                "y" => $datay,
            ],
        ],
    ];
}

function users_by_language()
{
    $res = DPDatabase::query("
        SELECT IFNULL(LEFT(u_intlang, 2), '') AS intlang, COUNT(*) AS num
        FROM users
        GROUP BY intlang
        ORDER BY num DESC
    ");
    $x = [];
    $y = [];

    while ($r = mysqli_fetch_assoc($res)) {
        array_push(
            $x,
            (
                $r['intlang'] ?
                dgettext("iso_639", eng_name($r['intlang'])) :
                _("Browser default")
            )
        );
        array_push($y, $r['num']);
    }

    $title = _("Number of users per user interface language");
    return [
        "title" => $title,
        "downloadLabel" => _("Download Chart"),
        "xAxisHeight" => 100,
        "data" => [
            _('Interface Language') => [
                "x" => $x,
                "y" => $y,
            ],
        ],
    ];
}

function users_by_country()
{
    $res = DPDatabase::query("
        SELECT SUBSTRING_INDEX(email, '.' ,-1) AS domain, COUNT(*) AS num
        FROM users
        WHERE email LIKE '%@%.%'
        GROUP BY domain
        ORDER BY num DESC
    ");
    $x = [];
    $y = [];

    while ($r = mysqli_fetch_assoc($res)) {
        array_push($x, $r['domain']);
        array_push($y, $r['num']);
    }

    $title = _("Number of users per country");
    return [
        "title" => $title,
        "downloadLabel" => _("Download Chart"),
        "data" => [
            _('Country') => [
                "x" => $x,
                "y" => $y,
            ],
        ],
    ];
}

/**
 * Return the array defined by $result[$i] = $arr[$i+1] - $arr[$i]
 */
function array_successive_differences($arr)
{
    $result = [];
    foreach ($arr as $key => $value) {
        if (isset($prev_key)) {
            $result[$prev_key] = $value - $prev_value;
        }
        $prev_key = $key;
        $prev_value = $value;
    }
    return $result;
}

function get_number_of_days_in_current_month()
{
    $current = getdate();
    // The last day of this month is the "zeroth" day of next month.
    $last_day_timestamp =
        mktime(0, 0, 0, $current['mon'] + 1, 0, $current['year']);
    $last_day = getdate($last_day_timestamp);
    return $last_day['mday'];
}

function curr_month_proj($which)
{
    // Create "projects Xed per day" graph for current month

    $psd = get_project_status_descriptor($which);

    $todaysTimeStamp = time();

    $year = date("Y", $todaysTimeStamp);
    $month = date("m", $todaysTimeStamp);
    $monthVar = _(date("F", $todaysTimeStamp));
    $timeframe = "$monthVar $year";

    $maxday = get_number_of_days_in_current_month();

    //query db and put results into arrays
    $sql = sprintf(
        "
        SELECT DAYOFMONTH(date) as day, SUM(num_projects)
        FROM project_state_stats
        WHERE MONTH(date) = %d AND YEAR(date) = %d AND (%s)
        GROUP BY DAYOFMONTH(date)
        ORDER BY date
        ",
        DPDatabase::escape($month),
        DPDatabase::escape($year),
        $psd->state_selector
    );
    $result = DPDatabase::query($sql);

    [$datax, $y_cumulative] = dpsql_fetch_columns($result);

    $datay1 = array_successive_differences($y_cumulative);

    // Pad out the rest of the month
    for ($i = count($datay1); $i < $maxday; $i++) {
        $datax[$i] = $i + 1;
        $datay1[$i] = 0;
    }

    return [
        "title" => "$psd->per_day_title ($timeframe)",
        "downloadLabel" => _("Download Chart"),
        "data" => [
            _('Projects') => [
                "x" => $datax,
                "y" => $datay1,
            ],
        ],
    ];
}

/**
 * Return the array defined by $result[$i] = $arr[$i] - $arr[0]
 */
function array_subtract_first_from_each($arr)
{
    $result = [];
    foreach ($arr as $key => $value) {
        if (count($result) == 0) {
            $first_value = $value;
        }
        $result[$key] = $value - $first_value;
    }
    return $result;
}

function cumulative_month_proj($which)
{
    // Create "projects Xed per day" graph for current month

    $psd = get_project_status_descriptor($which);

    $todaysTimeStamp = time();

    $year = date("Y", $todaysTimeStamp);
    $month = date("m", $todaysTimeStamp);
    $monthVar = _(date("F", $todaysTimeStamp));
    $timeframe = "$monthVar $year";

    $maxday = get_number_of_days_in_current_month();

    //query db and put results into arrays
    $sql = sprintf(
        "
        SELECT DAYOFMONTH(date) as day, SUM(num_projects)
        FROM project_state_stats
        WHERE MONTH(date) = %s AND YEAR(date) = %s AND (%s)
        GROUP BY DAYOFMONTH(date)
        ORDER BY date
        ",
        DPDatabase::escape($month),
        DPDatabase::escape($year),
        $psd->state_selector
    );
    $result = DPDatabase::query($sql);

    [$datax, $y_num_projects] = dpsql_fetch_columns($result);

    // get base level, total at beginning of 1st day of month
    // snapshot is taken just after midnight,
    // so day = 1 has total at beginning of month
    // Subtract that base level from each subsequent day's value
    $datay1 = array_subtract_first_from_each($y_num_projects);
    array_shift($datay1);

    // Pad out the rest of the month
    for ($i = count($datay1); $i < $maxday; $i++) {
        $datax[$i] = $i + 1;
        $datay1[$i] = "";
    }

    return [
        "title" => "$psd->cumulative_title ($timeframe)",
        "downloadLabel" => _("Download Chart"),
        "data" => [
            _('Projects') => [
                "x" => $datax,
                "y" => $datay1,
            ],
        ],
    ];
}

function total_proj_graph($which)
{
    //Create "projects Xed per day" graph for all known history

    $psd = get_project_status_descriptor($which);

    $timeframe = _('since stats began');


    //query db and put results into arrays
    $result = DPDatabase::query("
        SELECT date, SUM(num_projects)
        FROM project_state_stats
        WHERE $psd->state_selector
        GROUP BY date
        ORDER BY date
    ");
    [$datax, $y_cumulative] = dpsql_fetch_columns($result);

    $datay1 = array_successive_differences($y_cumulative);
    $datay1[] = 0;

    return [
        "title" => "$psd->per_day_title ($timeframe)",
        "downloadLabel" => _("Download Chart"),
        "xAxisHeight" => 75,
        "data" => [
            _('Projects') => [
                "x" => $datax,
                "y" => $datay1,
            ],
        ],
    ];
}

function cumulative_total_proj_graph($which)
{
    // Create "cumulative projects Xed per day" graph for all days
    // since state stats started being recorded up to yesterday

    $psd = get_project_status_descriptor($which);

    $timeframe = _('since stats began');

    //query db and put results into arrays
    $result = DPDatabase::query("
        SELECT date, SUM(num_projects)
        FROM project_state_stats
        WHERE $psd->state_selector
        GROUP BY date
        ORDER BY date ASC
    ");
    [$datax, $datay1] = dpsql_fetch_columns($result);

    if (empty($datay1)) {
        $datay1[0] = 0;
    }

    return [
        "title" => "$psd->cumulative_title ($timeframe)",
        "downloadLabel" => _("Download Chart"),
        "xAxisHeight" => 75,
        "data" => [
            _('Projects') => [
                "x" => $datax,
                "y" => $datay1,
            ],
        ],
    ];
}

function total_pages_by_month_graph($tally_name)
{
    ///////////////////////////////////////////////////
    //Total pages by month since beginning of stats

    $data = get_site_tally_grouped($tally_name, 'year_month');
    $datax = array_column($data, 0);
    $datay1 = array_column($data, 1);
    $datay2 = array_column($data, 2);

    return [
        "title" => _("Pages Done Each Month Since the Beginning of Statistics Collection"),
        "downloadLabel" => _("Download Chart"),
        "data" => [
            _("Actual") => [
                "x" => $datax,
                "y" => $datay1,
                "type" => "bar",
            ],
            _("Goal") => [
                "x" => $datax,
                "y" => $datay2,
                "type" => "line",
            ],
        ],
        "yAxisLabel" => _("Pages"),
        "legendAdjustment" => [
            "x" => 50,
            "y" => 15,
        ],
    ];
}

/**
 * Return the array defined by: $result[$i] = sum( $arr[$j] for $j up to and including $i )
 */
function array_accumulate($arr)
{
    $result = [];
    $sum = 0;
    foreach ($arr as $key => $value) {
        $sum += $value;
        $result[$key] = $sum;
    }
    return $result;
}

function pages_daily(string $tally_name, string $c_or_i, string $timeframe)
{
    $now_timestamp = time();
    $now_assoc = getdate($now_timestamp);

    $curr_y = $now_assoc['year'];
    $curr_m = $now_assoc['mon'];

    switch ($timeframe) {
        case 'curr_month':
            $start_timestamp = mktime(0, 0, 0, $curr_m, 1, $curr_y);
            $end_timestamp = mktime(0, 0, 0, $curr_m + 1, 1, $curr_y);
            $title_timeframe = icu_date('MMM y', $now_timestamp);
            break;

        case 'prev_month':
            $start_timestamp = mktime(0, 0, 0, $curr_m - 1, 1, $curr_y);
            $end_timestamp = mktime(0, 0, 0, $curr_m, 1, $curr_y);
            $title_timeframe = icu_date('MMM y', $start_timestamp);
            break;

        case 'all_time':
            $start_timestamp = 0;
            $end_timestamp = mktime(0, 0, 0, $curr_m + 1, 1, $curr_y);
            $title_timeframe = _('since stats began');
            break;

        default:
            throw new UnexpectedValueException("bad 'timeframe' value: '$timeframe'");
    }

    switch ($c_or_i) {
        case 'increments':
            $main_title = _('Pages Done Per Day');
            break;

        case 'cumulative':
            $main_title = _('Cumulative Pages Done');
            break;

        default:
            throw new UnexpectedValueException("bad 'cori' value: '$c_or_i'");
    }

    $days_to_average = 21;

    [$datax, $pages, $goal, $moving_average] = get_site_tally_cumulative_or_incremental(
        $tally_name,
        $c_or_i,
        $start_timestamp,
        $end_timestamp,
        $days_to_average
    );

    $data = [
        _("Actual") => [
            "x" => $datax,
            "y" => $pages,
            "type" => "bar",
        ],
        _("Goal") => [
            "x" => $datax,
            "y" => $goal ?? [],
            "type" => "line",
        ],
    ];

    if (!is_null($moving_average)) {
        $data[sprintf(_("%d-day SMA"), $days_to_average)] = [
            "x" => $datax,
            "y" => $moving_average,
            "type" => "line",
        ];
    }

    return [
        "title" => "$main_title ($title_timeframe)",
        "downloadLabel" => _("Download Chart"),
        "data" => $data,
        "yAxisLabel" => _("Pages"),
        "xAxisHeight" => 65,
        "legendAdjustment" => [
            "x" => 50,
            "y" => 15,
        ],
    ];
}

/**
 * Return an SQL selector that can be used in a WHERE clause.
 *
 * @param string $which
 *   A word denoting a round ID or a phase (NEW, PP, GB)
 */
function _get_project_state_selector($which, $desired_states = null)
{
    if (null == $desired_states) {
        $desired_states = ["unavailable", "waiting", "bad", "available", "complete"];
    } elseif (!is_array($desired_states)) {
        $desired_states = [$desired_states];
    }

    $sql = "";

    // see if it's a round
    $round = get_Round_for_round_id($which);
    if ($round != null) {
        $states = [];
        foreach ($desired_states as $desired_state) {
            $state_variable_name = "project_{$desired_state}_state";
            $states[] = $round->$state_variable_name;
        }
        $sql = "state IN ('" . implode("','", $states) . "')";
        return $sql;
    }

    // it may be a pool or stage so look at the stage's phase
    $states = [];
    foreach (ProjectStates::get_all() as $state => $project_state) {
        if ($project_state->phase == $which) {
            array_push($states, $state);
        }
    }
    if (count($states)) {
        $sql = "state IN ('" . implode("','", $states) . "')";
        return $sql;
    }

    throw new UnexpectedValueException("bad value for 'which': '$which'");
}

function get_round_backlog_stats($interested_phases)
{
    // Pull the stats data out of the database
    $stats = [];
    foreach ($interested_phases as $phase) {
        $where_state = _get_project_state_selector($phase, ["available", "waiting"]);
        $sql = "
            SELECT SUM(n_pages) as num_pages
            FROM projects
            WHERE $where_state
        ";
        $res = DPDatabase::query($sql);
        while ($result = mysqli_fetch_assoc($res)) {
            $stats[$phase] = $result["num_pages"];
        }
        mysqli_free_result($res);
    }

    // Pop off any trailing phases that have no pages.
    // This prevents new sites from showing rounds
    // that haven't started yet.
    $reverse_phases = array_reverse($interested_phases);
    foreach ($reverse_phases as $phase) {
        if ($stats[$phase] > 0) {
            break;
        }
        array_pop($stats);
    }

    return $stats;
}

//--------------------------------------------------------------------------

/**
 * Given a function and arguments that will generate graph data, attempt
 * to cache the value.
 *
 * Note that the graph function might include _() so it's important that we
 * include the user's language in the key.
 */
function query_graph_cache($function, $args = [], $expiration = 3600)
{
    return memoize_function($function, $args, $expiration, get_desired_language());
}
