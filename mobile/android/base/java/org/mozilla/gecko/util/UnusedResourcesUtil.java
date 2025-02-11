package org.mozilla.gecko.util;

import org.mozilla.gecko.R;

/**
 * (linter: UnusedResources) We use resources in places Android Lint can't check (e.g. JS) - this is
 * a set of those references so Android Lint stops complaining.
 */
@SuppressWarnings("unused")
final class UnusedResourcesUtil {
    public static final int[] CONSTANTS = {
            R.dimen.match_parent,
            R.dimen.wrap_content,
    };

    public static final int[] USED_IN_COLOR_PALETTE = {
            R.color.private_browsing_purple, // This will be used eventually, then this item removed.
    };

    public static final int[] USED_IN_JS = {
            R.drawable.ab_search,
            R.drawable.alert_camera,
            R.drawable.alert_download,
            R.drawable.alert_download_animation,
            R.drawable.alert_mic,
            R.drawable.alert_mic_camera,
            R.drawable.casting,
            R.drawable.casting_active,
            R.drawable.close,
            R.drawable.homepage_banner_firstrun,
            R.drawable.icon_openinapp,
            R.drawable.pause,
            R.drawable.phone,
            R.drawable.play,
            R.drawable.reader,
            R.drawable.reader_active,
            R.drawable.sync_promo,
            R.drawable.undo_button_icon,
    };

    public static final int[] USED_IN_MANIFEST = {
            R.drawable.search_launcher,
            R.xml.fxaccount_authenticator,
            R.xml.fxaccount_syncadapter,
            R.xml.search_widget_info,
            R.xml.searchable,
    };

    public static final int[] USED_IN_SUGGESTEDSITES = {
            R.drawable.suggestedsites_amazon,
            R.drawable.suggestedsites_facebook,
            R.drawable.suggestedsites_twitter,
            R.drawable.suggestedsites_wikipedia,
            R.drawable.suggestedsites_youtube,
    };

    public static final int[] USED_IN_BOOKMARKDEFAULTS = {
            R.raw.bookmarkdefaults_favicon_addons,
            R.raw.bookmarkdefaults_favicon_support,
            R.raw.bookmarkdefaults_favicon_restricted_support,
            R.raw.bookmarkdefaults_favicon_restricted_webmaker,
    };

    public static final int[] USED_IN_PREFS = {
            R.xml.preferences_advanced,
            R.xml.preferences_accessibility,
            R.xml.preferences_home,
            R.xml.preferences_privacy,
    };
}
