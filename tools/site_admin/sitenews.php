<?php
$relPath = "./../../pinc/";
include_once($relPath.'base.inc');
include_once($relPath.'theme.inc');
include_once($relPath.'user_is.inc');
include_once($relPath.'site_news.inc');

require_login();

if (!(user_is_a_sitemanager() or user_is_site_news_editor())) {
    die(_("You are not authorized to invoke this script."));
}

$news_page_id = get_enumerated_param($_GET, 'news_page_id', null, array_keys($NEWS_PAGES), true);
$action = get_enumerated_param($_GET, 'action', null, ['add', 'delete', 'display', 'hide', 'archive', 'unarchive', 'moveup', 'movedown', 'edit', 'edit_update'], true);
$item_id = get_integer_param($_REQUEST, 'item_id', null, null, null, true);
$header = $_POST['header'] ?? '';
$content = $_POST['content'] ?? '';
$locale_options = array_merge(['' => _("Any language")], get_locale_translation_selection_options());
$locale = get_enumerated_param($_POST, 'locale', null, array_keys($locale_options), true);
$status_options = [
    'current' => _("Sticky"),
    'recent' => _("Recent/Random"),
    'archived' => _("Archived"),
];
$item_status = get_enumerated_param($_POST, 'status', null, array_keys($status_options), true);
$item_type_options = [
    'normal' => _("Normal"),
    'announcement1' => _("Announcement - High"),
    'announcement2' => _("Announcement - Medium"),
    'announcement3' => _("Announcement - Low"),
    'celebration' => _("Celebration"),
    'maintenance' => _("Maintenance"),
];
$item_type = get_enumerated_param($_POST, 'item_type', null, array_keys($item_type_options), true);

if (isset($news_page_id)) {
    handle_any_requested_db_updates($news_page_id, $action, $item_id, $header, $content, $locale, $item_status, $item_type);

    $news_subject = get_news_subject($news_page_id);
    $title = sprintf(_('News Desk for %s'), $news_subject);
    output_header($title, NO_STATSBAR);
    output_page_links($news_page_id);
    echo "<h1>$title</h1>";

    $date_changed = get_news_page_last_modified_date($news_page_id);
    $last_modified = icu_date_template("long", $date_changed);
    echo "<p>" . _("Last modified") . ": ".$last_modified . "</p>";

    show_item_editor($news_page_id, $action, $item_id);
    show_all_news_items_for_page($news_page_id);
} else {
    $title = _("Site News Central");
    output_header($title, SHOW_STATSBAR);
    echo "<h1>$title</h1>";
    echo "<ul>";
    echo "\n";
    foreach ($NEWS_PAGES as $news_page_id => $news_subject) {
        echo "<li>";

        $news_subject = get_news_subject($news_page_id);
        $link = "<a href='sitenews.php?news_page_id=$news_page_id'>$news_subject</a>";
        echo sprintf(_("Edit Site News for %s"), $link);
        echo "\n";

        $date_changed = get_news_page_last_modified_date($news_page_id);
        if (!is_null($date_changed)) {
            $last_modified = icu_date_template("long", $date_changed);
            echo "<br>". _("Last modified").": ".$last_modified;
        }
        echo "<br><br>";
        echo "\n";
    }
    echo "</ul>";
}

// Everything else is just function declarations.

function output_page_links(string $current_page_id): void
{
    global $NEWS_PAGES;

    $links = [];
    $links[] = "<a href='sitenews.php'>" . _("Site News Central") . "</a>";
    foreach ($NEWS_PAGES as $news_page_id => $news_subject) {
        if ($news_page_id == $current_page_id) {
            $links[] = $news_page_id;
        } else {
            $links[] = "<a href='sitenews.php?news_page_id=$news_page_id'>$news_page_id</a>";
        }
    }
    echo "<p>" . implode(" | ", $links) . "</p>";
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

/** @param array<string, string> $options */
function echo_selection(string $name, array $options, string $value_selected): void
{
    echo "<select name='$name'>";
    foreach ($options as $value => $option) {
        $selected = '';
        if ($value_selected == $value) {
            $selected = ' selected';
        }
        echo "<option value='$value' $selected>$option</option>";
    }
    echo "</select>";
}

function handle_any_requested_db_updates(string $news_page_id, ?string $action, ?string $item_id, string $header, string $content, ?string $locale, ?string $item_status, ?string $item_type): void
{
    $allowed_tags = '<a><b><i><u><span><img><p><div><br><iframe>';
    switch ($action) {
        case 'add':
            // Save a new site news item
            $content = strip_tags($content, $allowed_tags);
            $sql = sprintf(
                "
                INSERT INTO news_items
                SET
                    id           = NULL,
                    news_page_id = LEFT('%s', 8),
                    date_posted  = %d,
                    status       = LEFT('%s', 8),
                    locale       = LEFT('%s', 8),
                    item_type    = LEFT('%s', 16),
                    header       = LEFT('%s', 256),
                    content      = '%s'
                ",
                DPDatabase::escape($news_page_id),
                time(),
                DPDatabase::escape($item_status),
                DPDatabase::escape($locale),
                DPDatabase::escape($item_type),
                DPDatabase::escape($header),
                DPDatabase::escape($content)
            );
            DPDatabase::query($sql);
            // by default, new items go at the top
            $sql = "
                UPDATE news_items
                SET ordering = id
                WHERE id = LAST_INSERT_ID()
            ";
            DPDatabase::query($sql);
            news_change_made($news_page_id);
            break;

        case 'delete':
            // Delete a specific site news item
            $sql = sprintf(
                "DELETE FROM news_items WHERE id = %d",
                $item_id
            );
            DPDatabase::query($sql);
            break;

        case 'display':
            // Display a specific site news item
            update_news_item_status($item_id, 'current');
            news_change_made($news_page_id);
            break;

        case 'hide':
            // Hide a specific site news item
            update_news_item_status($item_id, 'recent');
            news_change_made($news_page_id);
            break;

        case 'archive':
            // Archive a specific site news item
            update_news_item_status($item_id, 'archived');
            break;

        case 'unarchive':
            // Unarchive a specific site news item
            update_news_item_status($item_id, 'recent');
            break;

        case 'moveup':
            // Move a specific site news item higher in the display list
            move_news_item($news_page_id, $item_id, 'up');
            news_change_made($news_page_id);
            break;

        case 'movedown':
            // Move a specific site news item lower in the display list
            move_news_item($news_page_id, $item_id, 'down');
            news_change_made($news_page_id);
            break;

        case 'edit_update':
            // Save an update to a specific site news item
            $content = strip_tags($content, $allowed_tags);
            $sql = sprintf(
                "
                UPDATE news_items
                SET
                    status    = LEFT('%s', 8),
                    locale    = LEFT('%s', 8),
                    item_type = LEFT('%s', 16),
                    header    = LEFT('%s', 256),
                    content   = '%s'
                WHERE id = %d
                ",
                DPDatabase::escape($item_status),
                DPDatabase::escape($locale),
                DPDatabase::escape($item_type),
                DPDatabase::escape($header),
                DPDatabase::escape($content),
                $item_id
            );
            DPDatabase::query($sql);

            $sql = sprintf(
                "
                SELECT status
                FROM news_items
                WHERE id = %d
                ",
                $item_id
            );
            $result = DPDatabase::query($sql);
            $row = mysqli_fetch_assoc($result);
            $visible_change_made = ($row['status'] == 'current');
            if ($visible_change_made) {
                news_change_made($news_page_id);
            }
            break;
    }
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

/**
 * Show a form to edit a news item
 */
function show_item_editor(string $news_page_id, ?string $action, ?string $item_id): void
{
    global $locale_options;
    global $status_options;
    global $item_type_options;

    if (isset($action) && $action == "edit") {
        $sql = sprintf(
            "
            SELECT header, content, status, locale, item_type
            FROM news_items
            WHERE id = %d
            ",
            $item_id
        );
        $result = DPDatabase::query($sql);
        $row = mysqli_fetch_assoc($result);
        $header = $row["header"];
        $initial_content = $row["content"];
        $locale = $row["locale"];
        $item_status = $row['status'];
        $item_type = $row['item_type'];
        $action_to_request = "edit_update";
        $submit_button_label = _("Save News Item");
        $title = _("Edit News Item");
    } else {
        $item_id = 0;
        $header = "";
        $initial_content = "";
        $locale = "";
        $item_status = 'current';
        $item_type = 'normal';
        $action_to_request = "add";
        $submit_button_label = _("Add News Item");
        $title = _("Add News Item");
    }

    echo "<h2>$title</h2>";

    echo "<form action='sitenews.php?news_page_id=$news_page_id&action=$action_to_request' method='post'>";
    echo "<input type='hidden' name='item_id' value='$item_id'>";
    echo "<table class='newsedit'>";
    echo "<tr>";
    echo "<td class='commands'><b>" . _("Show to users using") . "</b></td>";
    echo "<td class='items'>";
    echo_selection("locale", $locale_options, $locale);
    echo "</td>";
    echo "</tr>";
    echo "<tr>";
    echo "<td class='commands'><b>" . _("Status") . "</b></td>";
    echo "<td class='items'>";
    echo_selection("status", $status_options, $item_status);
    echo "</td>";
    echo "</tr>";
    echo "<tr>";
    echo "<td class='commands'><b>" . _("News Type") . "</b></td>";
    echo "<td class='items'>";
    echo_selection("item_type", $item_type_options, $item_type);
    echo "</td>";
    echo "</tr>";
    echo "<tr>";
    echo "<td class='commands'><b>" . _("News Header") . "</b></td>";
    echo "<td class='items'><input name='header' style='width:100%;' value='" . attr_safe($header) . "'></td>";
    echo "</tr>";
    echo "<tr>";
    echo "<td class='commands'><b>" . _("News Item") . "</b></td>";
    echo "<td class='items'><textarea name='content' style='width:100%;height:9em;'>" . html_safe($initial_content) . "</textarea></td>";
    echo "</tr>";
    echo "<tr>";
    echo "<td></td><td><input type='submit' value='$submit_button_label' name='submit'></td>";
    echo "</tr>";
    echo "</table>";
    echo "</form>";
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

function show_all_news_items_for_page(string $news_page_id): void
{
    // three categories:
    // 1) current  (currently displayed on page every time)
    // 2) recent   (displayed on "Recent News", and one shown as Random)
    // 3) archived (not visible to users at all, saved for later use or historical interest)

    $categories = [
        [
            'status' => 'current',
            'title' => _('Sticky News Items'),
            'blurb' => _("All of these items are shown every time the page is loaded. Most important and recent news items go here, where they are guaranteed to be displayed."),
            'order_by' => 'ordering DESC',
            'actions' => [
                'hide' => _('Unstick'),
                'archive' => _('Archive'),
                'moveup' => _('Move Up'),
                'movedown' => _('Move Down'),
            ],
        ],

        [
            'status' => 'recent',
            'title' => _('Random/Recent News Items'),
            'blurb' => _("This is the pool of available random news items for this page. Every time the page is loaded, a randomly selected one of these items is displayed."),
            'order_by' => 'ordering DESC',
            'actions' => [
                'display' => _('Stick'),
                'archive' => _('Archive'),
            ],
        ],

        [
            'status' => 'archived',
            'title' => _('Archived News Items'),
            'blurb' => _("Items here are not visible anywhere, and can be safely stored here until they become current again."),
            'order_by' => 'id DESC',
            'actions' => [
                'unarchive' => _('Unarchive'),
            ],
        ],
    ];

    foreach ($categories as $category) {
        $status = $category['status'];

        $sql = sprintf(
            "
            SELECT *
            FROM news_items
            WHERE news_page_id = '%s' AND status = '%s'
            ORDER BY {$category['order_by']}
            ",
            DPDatabase::escape($news_page_id),
            DPDatabase::escape($status)
        );
        $result = DPDatabase::query($sql);

        if (mysqli_num_rows($result) == 0) {
            continue;
        }

        echo "<h2>{$category['title']}</h2>";
        echo "<p>" . $category['blurb'] . "</p>\n";

        $actions = $category['actions'] +
            [
                'edit' => _('Edit'),
                'delete' => _('Delete'),
            ];

        echo "<table class='newsedit'>";
        while ($news_item = mysqli_fetch_array($result)) {
            echo "<tbody class='padding'>";
            echo "<tr>";
            echo "<td class='commands'>";
            echo "<p><b>" . _("Posted") . ":</b><br>";
            echo icu_date_template("short", $news_item['date_posted']) . "</b>";
            if ($news_item['locale']) {
                echo "<br><b>" . _("Locale") . ":</b><br>" . $news_item['locale'];
            }
            echo "</p>";
            foreach ($actions as $action => $label) {
                $url = "sitenews.php?news_page_id=$news_page_id&item_id={$news_item['id']}&action=$action";
                if ($action == "delete") {
                    $onclick = "onclick='return confirm(\"" . javascript_safe(_("Delete this news item?")) . "\");'";
                } else {
                    $onclick = '';
                }
                echo "<a href='$url' $onclick>$label</a><br>";
            }
            echo "</td>";
            echo "<td class='items default-border-bottom'>";
            echo "<div class='news-header news-{$news_item['item_type']}'>" . html_safe($news_item['header']) . "</div>";
            echo "\n";
            echo $news_item['content'];
            echo "\n";
            echo "</td>";
            echo "</tr>";
            echo "</tbody>";
        }
        echo "</table>";
    }
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

function update_news_item_status(string $item_id, string $status): void
{
    $sql = sprintf(
        "
        UPDATE news_items
        SET status = '%s'
        WHERE id = %d
        ",
        DPDatabase::escape($status),
        $item_id
    );
    DPDatabase::query($sql);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

function news_change_made(string $news_page_id): void
{
    $sql = sprintf(
        "
        REPLACE INTO news_pages
        SET news_page_id = '%s', t_last_change = %d
        ",
        DPDatabase::escape($news_page_id),
        time()
    );
    DPDatabase::query($sql);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

function move_news_item(string $news_page_id, string $id_of_item_to_move, string $direction): void
{
    $sql = sprintf(
        "
        SELECT *
        FROM news_items
        WHERE news_page_id = '%s' AND status = 'current'
        ORDER BY ordering
        ",
        DPDatabase::escape($news_page_id)
    );
    $result = DPDatabase::query($sql);

    $i = 1 ;
    while ($news_item = mysqli_fetch_assoc($result)) {
        $curr_id = $news_item['id'];
        $sql = sprintf(
            "
            UPDATE news_items
            SET ordering = %d
            WHERE id = %d
            ",
            $i,
            $curr_id
        );
        DPDatabase::query($sql);
        if (intval($curr_id) == intval($id_of_item_to_move)) {
            $old_pos = $i;
        }
        $i++;
    }

    if (isset($old_pos)) {
        if ($direction == 'up') {
            $new_pos = $old_pos + 1;
        } else {
            $new_pos = $old_pos - 1;
        }

        $sql = sprintf(
            "
            UPDATE news_items
            SET ordering = %d
            WHERE news_page_id = '%s' AND status = 'current' AND ordering = $new_pos
            ",
            $old_pos,
            DPDatabase::escape($news_page_id),
            $new_pos
        );
        DPDatabase::query($sql);

        $sql = sprintf(
            "
            UPDATE news_items
            SET ordering = %d
            WHERE id = %d
            ",
            $new_pos,
            $id_of_item_to_move
        );
        DPDatabase::query($sql);
    }
}
