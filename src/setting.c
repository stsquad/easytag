/* config.c - 2000/06/21 */
/*
 *  EasyTAG - Tag editor for MP3 and Ogg Vorbis files
 *  Copyright (C) 2000-2003  Jerome Couderc <easytag@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "setting.h"
#include "prefs.h"
#include "bar.h"
#include "easytag.h"
#include "charset.h"
#include "scan.h"
#include "log.h"
#include "misc.h"
#include "cddb.h"
#include "browser.h"

#ifdef WIN32
#   include "win32/win32dep.h"
#endif


/***************
 * Declaration *
 ***************/

/*
 * Nota :
 *  - no trailing slashes on directory name to avoid problem with
 *    NetBSD's mkdir(2).
 */

// Base directory created into home dir
#define EASYTAG_DIR                                 ".easytag"
// File for configuration
#define CONFIG_FILE                                 EASYTAG_DIR G_DIR_SEPARATOR_S "easytagrc"
// File of masks for tag scanner
#define SCAN_TAG_MASKS_FILE                         EASYTAG_DIR G_DIR_SEPARATOR_S "scan_tag.mask"
// File of masks for rename file scanner
#define RENAME_FILE_MASKS_FILE                      EASYTAG_DIR G_DIR_SEPARATOR_S "rename_file.mask"
// File for history of RenameDirectoryMaskCombo combobox
#define RENAME_DIRECTORY_MASKS_FILE                 EASYTAG_DIR G_DIR_SEPARATOR_S "rename_directory.mask"
// File for history of PlayListNameCombo combobox
#define PLAY_LIST_NAME_MASKS_FILE                   EASYTAG_DIR G_DIR_SEPARATOR_S "play_list_name.mask"
// File for history of PlayListContentMaskEntry combobox
#define PLAYLIST_CONTENT_MASKS_FILE                 EASYTAG_DIR G_DIR_SEPARATOR_S "playlist_content.mask"
// File for history of DefaultPathToMp3 combobox
#define DEFAULT_PATH_TO_MP3_HISTORY_FILE            EASYTAG_DIR G_DIR_SEPARATOR_S "default_path_to_mp3.history"
// File for history of DefaultComment combobox
#define DEFAULT_TAG_COMMENT_HISTORY_FILE            EASYTAG_DIR G_DIR_SEPARATOR_S "default_tag_comment.history"
// File for history of BrowserEntry combobox
#define PATH_ENTRY_HISTORY_FILE                     EASYTAG_DIR G_DIR_SEPARATOR_S "browser_path.history"
// File for history of run program combobox for directories
#define RUN_PROGRAM_WITH_DIRECTORY_HISTORY_FILE     EASYTAG_DIR G_DIR_SEPARATOR_S "run_program_with_directory.history"
// File for history of run program combobox for files
#define RUN_PROGRAM_WITH_FILE_HISTORY_FILE          EASYTAG_DIR G_DIR_SEPARATOR_S "run_program_with_file.history"
// File for history of run player combobox
#define AUDIO_FILE_PLAYER_HISTORY_FILE              EASYTAG_DIR G_DIR_SEPARATOR_S "audio_file_player.history"
// File for history of search string combobox
#define SEARCH_FILE_HISTORY_FILE                    EASYTAG_DIR G_DIR_SEPARATOR_S "search_file.history"
// File for history of FileToLoad combobox
#define FILE_TO_LOAD_HISTORY_FILE                   EASYTAG_DIR G_DIR_SEPARATOR_S "file_to_load.history"
// File for history of CddbSearchStringEntry combobox
#define CDDB_SEARCH_STRING_HISTORY_FILE             EASYTAG_DIR G_DIR_SEPARATOR_S "cddb_search_string.history"
// File for history of CddbSearchStringInResultEntry combobox
#define CDDB_SEARCH_STRING_IN_RESULT_HISTORY_FILE   EASYTAG_DIR G_DIR_SEPARATOR_S "cddb_search_string_in_result.history"
// File for history of CddbLocalPath combobox
#define CDDB_LOCAL_PATH_HISTORY_FILE                EASYTAG_DIR G_DIR_SEPARATOR_S "cddb_local_path.history"



/**************
 * Prototypes *
 **************/



/********************
 * Config Variables *
 ********************/
tConfigVariable Config_Variables[] =
{
    {"load_on_startup",                     CV_TYPE_BOOL,    &LOAD_ON_STARTUP                   },
    {"default_path_to_mp3",                 CV_TYPE_STRING,  &DEFAULT_PATH_TO_MP3               },
    {"browser_line_style",                  CV_TYPE_BOOL,    &BROWSER_LINE_STYLE                },
    {"browser_expander_style",              CV_TYPE_BOOL,    &BROWSER_EXPANDER_STYLE            },
    {"browse_subdir",                       CV_TYPE_BOOL,    &BROWSE_SUBDIR                     },
    {"browse_hidden_dir",                   CV_TYPE_BOOL,    &BROWSE_HIDDEN_DIR                 },
    {"open_selected_browser_node",          CV_TYPE_BOOL,    &OPEN_SELECTED_BROWSER_NODE        },

    {"set_main_window_position",            CV_TYPE_BOOL,    &SET_MAIN_WINDOW_POSITION          },
    {"main_window_x",                       CV_TYPE_INT,     &MAIN_WINDOW_X                     },
    {"main_window_y",                       CV_TYPE_INT,     &MAIN_WINDOW_Y                     },
    {"main_window_height",                  CV_TYPE_INT,     &MAIN_WINDOW_HEIGHT                },
    {"main_window_width",                   CV_TYPE_INT,     &MAIN_WINDOW_WIDTH                 },
    {"pane_handle_position1",               CV_TYPE_INT,     &PANE_HANDLE_POSITION1             },
    {"pane_handle_position2",               CV_TYPE_INT,     &PANE_HANDLE_POSITION2             },
    {"pane_handle_position3",               CV_TYPE_INT,     &PANE_HANDLE_POSITION3             },
    {"pane_handle_position4",               CV_TYPE_INT,     &PANE_HANDLE_POSITION4             },
    {"show_header_infos",                   CV_TYPE_BOOL,    &SHOW_HEADER_INFO                  },
    {"changed_files_displayed_to_red",      CV_TYPE_BOOL,    &CHANGED_FILES_DISPLAYED_TO_RED    },
    {"changed_files_displayed_to_bold",     CV_TYPE_BOOL,    &CHANGED_FILES_DISPLAYED_TO_BOLD   },

    {"date_auto_completion",                 CV_TYPE_BOOL,    &DATE_AUTO_COMPLETION                     },
    {"number_track_formated",                CV_TYPE_BOOL,    &NUMBER_TRACK_FORMATED                    },
    {"number_track_formated_spin_button",    CV_TYPE_INT,     &NUMBER_TRACK_FORMATED_SPIN_BUTTON        },
    {"ogg_tag_write_xmms_comment",           CV_TYPE_BOOL,    &OGG_TAG_WRITE_XMMS_COMMENT               },
    {"set_focus_to_same_tag_field",          CV_TYPE_BOOL,    &SET_FOCUS_TO_SAME_TAG_FIELD              },
    {"set_focus_to_first_tag_field",         CV_TYPE_BOOL,    &SET_FOCUS_TO_FIRST_TAG_FIELD             },
    {"sorting_file_mode",                    CV_TYPE_INT,     &SORTING_FILE_MODE                        },
    {"sorting_file_case_sensitive",          CV_TYPE_BOOL,    &SORTING_FILE_CASE_SENSITIVE              },
    {"log_max_lines",                        CV_TYPE_INT,     &LOG_MAX_LINES                            },
    {"sho_log_view",                         CV_TYPE_BOOL,    &SHOW_LOG_VIEW                            },

    {"replace_illegal_character_in_filename",          CV_TYPE_BOOL,    &REPLACE_ILLEGAL_CHARACTERS_IN_FILENAME   },
    {"filename_extension_lower_case",                  CV_TYPE_BOOL,    &FILENAME_EXTENSION_LOWER_CASE            },
    {"filename_extension_upper_case",                  CV_TYPE_BOOL,    &FILENAME_EXTENSION_UPPER_CASE            },
    {"filename_extension_no_change",                   CV_TYPE_BOOL,    &FILENAME_EXTENSION_NO_CHANGE             },
    {"preserve_modification_time",                     CV_TYPE_BOOL,    &PRESERVE_MODIFICATION_TIME               },
    {"update_parent_directory_modification_time",      CV_TYPE_BOOL,    &UPDATE_PARENT_DIRECTORY_MODIFICATION_TIME},
    {"filename_character_set_other",                   CV_TYPE_BOOL,    &FILENAME_CHARACTER_SET_OTHER             },
    {"filename_character_set_approximate",             CV_TYPE_BOOL,    &FILENAME_CHARACTER_SET_APPROXIMATE       },
    {"filename_character_set_discard",                 CV_TYPE_BOOL,    &FILENAME_CHARACTER_SET_DISCARD           },

    {"write_id3_tags_in_flac_file",                    CV_TYPE_BOOL,  &WRITE_ID3_TAGS_IN_FLAC_FILE                     },
    {"strip_tag_when_empty_fields",                    CV_TYPE_BOOL,  &STRIP_TAG_WHEN_EMPTY_FIELDS                     },
    {"convert_old_id3v2_tag_version",                  CV_TYPE_BOOL,  &CONVERT_OLD_ID3V2_TAG_VERSION                   },
    {"use_non_standard_id3_reading_character_set",     CV_TYPE_BOOL,  &USE_NON_STANDARD_ID3_READING_CHARACTER_SET},
    {"file_reading_id3v1v2_character_set",             CV_TYPE_STRING,&FILE_READING_ID3V1V2_CHARACTER_SET},
    {"file_writing_id3v2_write_tag",                   CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_WRITE_TAG    },
    {"file_writing_id3v2_version_4",                   CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_VERSION_4   },
    {"file_writing_id3v2_use_crc32",                   CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_USE_CRC32    },
    {"file_writing_id3v2_use_compression",             CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_USE_COMPRESSION    },
    {"file_writing_id3v2_use_unicode_character_set",   CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_USE_UNICODE_CHARACTER_SET},
    {"file_writing_id3v2_unicode_character_set",       CV_TYPE_STRING,&FILE_WRITING_ID3V2_UNICODE_CHARACTER_SET},
    {"file_writing_id3v2_no_unicode_character_set",    CV_TYPE_STRING,&FILE_WRITING_ID3V2_NO_UNICODE_CHARACTER_SET},
    {"file_writing_id3v2_iconv_options_no",            CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_ICONV_OPTIONS_NO},
    {"file_writing_id3v2_iconv_options_translit",      CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_ICONV_OPTIONS_TRANSLIT},
    {"file_writing_id3v2_iconv_options_ignore",        CV_TYPE_BOOL,  &FILE_WRITING_ID3V2_ICONV_OPTIONS_IGNORE},
    {"file_writing_id3v1_write_tag",                   CV_TYPE_BOOL,  &FILE_WRITING_ID3V1_WRITE_TAG   },
    {"file_writing_id3v1_character_set",               CV_TYPE_STRING,&FILE_WRITING_ID3V1_CHARACTER_SET},
    {"file_writing_id3v1_iconv_options_no",            CV_TYPE_BOOL,  &FILE_WRITING_ID3V1_ICONV_OPTIONS_NO},
    {"file_writing_id3v1_iconv_options_translit",      CV_TYPE_BOOL,  &FILE_WRITING_ID3V1_ICONV_OPTIONS_TRANSLIT},
    {"file_writing_id3v1_iconv_options_ignore",        CV_TYPE_BOOL,  &FILE_WRITING_ID3V1_ICONV_OPTIONS_IGNORE},
    {"vorbis_split_field_title",                       CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_TITLE},
    {"vorbis_split_field_artist",                      CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_ARTIST},
    {"vorbis_split_field_album",                       CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_ALBUM},
    {"vorbis_split_field_genre",                       CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_GENRE},
    {"vorbis_split_field_comment",                     CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_COMMENT},
    {"vorbis_split_field_composer",                    CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_COMPOSER},
    {"vorbis_split_field_orig_artist",                 CV_TYPE_BOOL,  &VORBIS_SPLIT_FIELD_ORIG_ARTIST},

    {"message_box_position_none",               CV_TYPE_BOOL,  &MESSAGE_BOX_POSITION_NONE                },
    {"message_box_position_center",             CV_TYPE_BOOL,  &MESSAGE_BOX_POSITION_CENTER              },
    {"message_box_position_mouse",              CV_TYPE_BOOL,  &MESSAGE_BOX_POSITION_MOUSE               },
    {"message_box_position_center_on_parent",   CV_TYPE_BOOL,  &MESSAGE_BOX_POSITION_CENTER_ON_PARENT    },

    {"audio_file_player",                       CV_TYPE_STRING,&AUDIO_FILE_PLAYER                        },

    {"scanner_type",                             CV_TYPE_INT, &SCANNER_TYPE                              },
    {"scan_mask_editor_button",                  CV_TYPE_BOOL,&SCAN_MASK_EDITOR_BUTTON                   },
    {"scan_legend_button",                       CV_TYPE_BOOL,&SCAN_LEGEND_BUTTON                        },
    {"fts_convert_underscore_and_p20_into_space",CV_TYPE_BOOL,&FTS_CONVERT_UNDERSCORE_AND_P20_INTO_SPACE },
    {"fts_convert_space_into_underscore",        CV_TYPE_BOOL,&FTS_CONVERT_SPACE_INTO_UNDERSCORE         },
    {"rfs_convert_underscore_and_p20_into_space",CV_TYPE_BOOL,&RFS_CONVERT_UNDERSCORE_AND_P20_INTO_SPACE },
    {"rfs_convert_space_into_underscore",        CV_TYPE_BOOL,&RFS_CONVERT_SPACE_INTO_UNDERSCORE         },
    {"pfs_dont_upper_some_words",                CV_TYPE_BOOL,&PFS_DONT_UPPER_SOME_WORDS                 },
    {"overwrite_tag_field",                     CV_TYPE_BOOL,    &OVERWRITE_TAG_FIELD                    },
    {"set_default_comment",                     CV_TYPE_BOOL,    &SET_DEFAULT_COMMENT                    },
    {"default_comment",                         CV_TYPE_STRING,  &DEFAULT_COMMENT                        },
    {"crc32_comment",                           CV_TYPE_BOOL,    &SET_CRC32_COMMENT                      },
    {"open_scanner_window_on_startup",          CV_TYPE_BOOL,    &OPEN_SCANNER_WINDOW_ON_STARTUP         },
    {"scanner_window_on_top",                   CV_TYPE_BOOL,    &SCANNER_WINDOW_ON_TOP                  },
    {"set_scanner_window_position",             CV_TYPE_BOOL,    &SET_SCANNER_WINDOW_POSITION            },
    {"scanner_window_x",                        CV_TYPE_INT,     &SCANNER_WINDOW_X                       },
    {"scanner_window_y",                        CV_TYPE_INT,     &SCANNER_WINDOW_Y                       },

    {"confirm_before_exit",                     CV_TYPE_BOOL,    &CONFIRM_BEFORE_EXIT                    },
    {"confirm_write_tag",                       CV_TYPE_BOOL,    &CONFIRM_WRITE_TAG                      },
    {"confirm_rename_file",                     CV_TYPE_BOOL,    &CONFIRM_RENAME_FILE                    },
    {"confirm_write_playlist",                  CV_TYPE_BOOL,    &CONFIRM_WRITE_PLAYLIST                 },
    {"confirm_delete_file",                     CV_TYPE_BOOL,    &CONFIRM_DELETE_FILE                    },
    {"process_filename_field",                  CV_TYPE_BOOL,    &PROCESS_FILENAME_FIELD                 },
    {"process_title_field",                     CV_TYPE_BOOL,    &PROCESS_TITLE_FIELD                    },
    {"process_artist_field",                    CV_TYPE_BOOL,    &PROCESS_ARTIST_FIELD                   },
    {"process_album_artist_field",              CV_TYPE_BOOL,    &PROCESS_ALBUM_ARTIST_FIELD             },
    {"process_album_field",                     CV_TYPE_BOOL,    &PROCESS_ALBUM_FIELD                    },
    {"process_genre_field",                     CV_TYPE_BOOL,    &PROCESS_GENRE_FIELD                    },
    {"process_comment_field",                   CV_TYPE_BOOL,    &PROCESS_COMMENT_FIELD                  },
    {"process_composer_field",                  CV_TYPE_BOOL,    &PROCESS_COMPOSER_FIELD                 },
    {"process_orig_artist_field",               CV_TYPE_BOOL,    &PROCESS_ORIG_ARTIST_FIELD              },
    {"process_copyright_field",                 CV_TYPE_BOOL,    &PROCESS_COPYRIGHT_FIELD                },
    {"process_url_field",                       CV_TYPE_BOOL,    &PROCESS_URL_FIELD                      },
    {"process_encoded_by_field",                CV_TYPE_BOOL,    &PROCESS_ENCODED_BY_FIELD               },
    {"process_fields_convert_from",             CV_TYPE_STRING,  &PROCESS_FIELDS_CONVERT_FROM            },
    {"process_fields_convert_to",               CV_TYPE_STRING,  &PROCESS_FIELDS_CONVERT_TO              },

    {"pf_convert_into_space",                   CV_TYPE_BOOL,    &PF_CONVERT_INTO_SPACE                  },
    {"pf_convert_space",                        CV_TYPE_BOOL,    &PF_CONVERT_SPACE                       },
    {"pf_convert",                              CV_TYPE_BOOL,    &PF_CONVERT                             },
    {"pf_convert_all_uppercase",                CV_TYPE_BOOL,    &PF_CONVERT_ALL_UPPERCASE               },
    {"pf_convert_all_downcase",                 CV_TYPE_BOOL,    &PF_CONVERT_ALL_DOWNCASE                },
    {"pf_convert_first_letter_uppercase",       CV_TYPE_BOOL,    &PF_CONVERT_FIRST_LETTER_UPPERCASE      },
    {"pf_convert_first_letters_uppercase",      CV_TYPE_BOOL,    &PF_CONVERT_FIRST_LETTERS_UPPERCASE     },
    {"pf_detect_roman_numerals",                CV_TYPE_BOOL,    &PF_DETECT_ROMAN_NUMERALS               },
    {"pf_remove_space",                         CV_TYPE_BOOL,    &PF_REMOVE_SPACE                        },
    {"pf_insert_space",                         CV_TYPE_BOOL,    &PF_INSERT_SPACE                        },
    {"pf_only_one_space",                       CV_TYPE_BOOL,    &PF_ONLY_ONE_SPACE                      },

    {"playlist_name",                           CV_TYPE_STRING,  &PLAYLIST_NAME                          },
    {"playlist_use_mask_name",                  CV_TYPE_BOOL,    &PLAYLIST_USE_MASK_NAME                 },
    {"playlist_use_dir_name",                   CV_TYPE_BOOL,    &PLAYLIST_USE_DIR_NAME                  },
    {"playlist_only_selected_files",            CV_TYPE_BOOL,    &PLAYLIST_ONLY_SELECTED_FILES           },
    {"playlist_full_path",                      CV_TYPE_BOOL,    &PLAYLIST_FULL_PATH                     },
    {"playlist_relative_path",                  CV_TYPE_BOOL,    &PLAYLIST_RELATIVE_PATH                 },
    {"playlist_create_in_parent_dir",           CV_TYPE_BOOL,    &PLAYLIST_CREATE_IN_PARENT_DIR          },
    {"playlist_use_dos_separator",              CV_TYPE_BOOL,    &PLAYLIST_USE_DOS_SEPARATOR             },
    {"playlist_content_none",                   CV_TYPE_BOOL,    &PLAYLIST_CONTENT_NONE                  },
    {"playlist_content_filename",               CV_TYPE_BOOL,    &PLAYLIST_CONTENT_FILENAME              },
    {"playlist_content_mask",                   CV_TYPE_BOOL,    &PLAYLIST_CONTENT_MASK                  },
    {"playlist_content_mask_value",             CV_TYPE_STRING,  &PLAYLIST_CONTENT_MASK_VALUE            },
    {"playlist_window_x",                       CV_TYPE_INT,     &PLAYLIST_WINDOW_X                      },
    {"playlist_window_y",                       CV_TYPE_INT,     &PLAYLIST_WINDOW_Y                      },
    {"playlist_window_width",                   CV_TYPE_INT,     &PLAYLIST_WINDOW_WIDTH                  },
    {"playlist_window_height",                  CV_TYPE_INT,     &PLAYLIST_WINDOW_HEIGHT                 },

    {"load_file_run_scanner",                   CV_TYPE_BOOL,    &LOAD_FILE_RUN_SCANNER                  },
    {"load_file_window_x",                      CV_TYPE_INT,     &LOAD_FILE_WINDOW_X                     },
    {"load_file_window_y",                      CV_TYPE_INT,     &LOAD_FILE_WINDOW_Y                     },
    {"load_file_window_width",                  CV_TYPE_INT,     &LOAD_FILE_WINDOW_WIDTH                 },
    {"load_file_window_height",                 CV_TYPE_INT,     &LOAD_FILE_WINDOW_HEIGHT                },

    {"cddb_server_name_automatic_search",       CV_TYPE_STRING,  &CDDB_SERVER_NAME_AUTOMATIC_SEARCH      },
    {"cddb_server_port_automatic_search",       CV_TYPE_INT,     &CDDB_SERVER_PORT_AUTOMATIC_SEARCH      },
    {"cddb_server_cgi_path_automatic_search",   CV_TYPE_STRING,  &CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH  },
    {"cddb_server_name_automatic_search2",      CV_TYPE_STRING,  &CDDB_SERVER_NAME_AUTOMATIC_SEARCH2     },
    {"cddb_server_port_automatic_search2",      CV_TYPE_INT,     &CDDB_SERVER_PORT_AUTOMATIC_SEARCH2     },
    {"cddb_server_cgi_path_automatic_search2",  CV_TYPE_STRING,  &CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH2 },
    {"cddb_server_name_manual_search",          CV_TYPE_STRING,  &CDDB_SERVER_NAME_MANUAL_SEARCH         },
    {"cddb_server_port_manual_search",          CV_TYPE_INT,     &CDDB_SERVER_PORT_MANUAL_SEARCH         },
    {"cddb_server_cgi_path_manual_search",      CV_TYPE_STRING,  &CDDB_SERVER_CGI_PATH_MANUAL_SEARCH     },
    {"cddb_local_path",                         CV_TYPE_STRING,  &CDDB_LOCAL_PATH                        },
    {"cddb_use_proxy",                          CV_TYPE_INT,     &CDDB_USE_PROXY                         },
    {"cddb_proxy_name",                         CV_TYPE_STRING,  &CDDB_PROXY_NAME                        },
    {"cddb_proxy_port",                         CV_TYPE_INT,     &CDDB_PROXY_PORT                        },
    {"cddb_proxy_user_name",                    CV_TYPE_STRING,  &CDDB_PROXY_USER_NAME                   },
    {"cddb_proxy_user_password",                CV_TYPE_STRING,  &CDDB_PROXY_USER_PASSWORD               },
    {"set_cddb_window_position",                CV_TYPE_BOOL,    &SET_CDDB_WINDOW_POSITION               },
    {"cddb_window_x",                           CV_TYPE_INT,     &CDDB_WINDOW_X                          },
    {"cddb_window_y",                           CV_TYPE_INT,     &CDDB_WINDOW_Y                          },
    {"cddb_window_height",                      CV_TYPE_INT,     &CDDB_WINDOW_HEIGHT                     },
    {"cddb_window_width",                       CV_TYPE_INT,     &CDDB_WINDOW_WIDTH                      },
    {"cddb_pane_handle_position",               CV_TYPE_INT,     &CDDB_PANE_HANDLE_POSITION              },

    {"cddb_follow_file",                        CV_TYPE_BOOL,    &CDDB_FOLLOW_FILE                       },
    {"cddb_use_dlm",                            CV_TYPE_BOOL,    &CDDB_USE_DLM                           },
    {"cddb_use_local_access",                   CV_TYPE_BOOL,    &CDDB_USE_LOCAL_ACCESS                  },

    {"cddb_search_in_all_fields",               CV_TYPE_BOOL,    &CDDB_SEARCH_IN_ALL_FIELDS              },
    {"cddb_search_in_artist_field",             CV_TYPE_BOOL,    &CDDB_SEARCH_IN_ARTIST_FIELD            },
    {"cddb_search_in_title_field",              CV_TYPE_BOOL,    &CDDB_SEARCH_IN_TITLE_FIELD             },
    {"cddb_search_in_track_name_field",         CV_TYPE_BOOL,    &CDDB_SEARCH_IN_TRACK_NAME_FIELD        },
    {"cddb_search_in_other_field",              CV_TYPE_BOOL,    &CDDB_SEARCH_IN_OTHER_FIELD             },
    {"cddb_show_categories",                    CV_TYPE_BOOL,    &CDDB_SHOW_CATEGORIES                   },

    {"cddb_search_in_all_categories",           CV_TYPE_BOOL,    &CDDB_SEARCH_IN_ALL_CATEGORIES          },
    {"cddb_search_in_blues_categories",         CV_TYPE_BOOL,    &CDDB_SEARCH_IN_BLUES_CATEGORY          },
    {"cddb_search_in_classical_categories",     CV_TYPE_BOOL,    &CDDB_SEARCH_IN_CLASSICAL_CATEGORY      },
    {"cddb_search_in_country_categories",       CV_TYPE_BOOL,    &CDDB_SEARCH_IN_COUNTRY_CATEGORY        },
    {"cddb_search_in_folk_categories",          CV_TYPE_BOOL,    &CDDB_SEARCH_IN_FOLK_CATEGORY           },
    {"cddb_search_in_jazz_categories",          CV_TYPE_BOOL,    &CDDB_SEARCH_IN_JAZZ_CATEGORY           },
    {"cddb_search_in_misc_categories",          CV_TYPE_BOOL,    &CDDB_SEARCH_IN_MISC_CATEGORY           },
    {"cddb_search_in_newage_categories",        CV_TYPE_BOOL,    &CDDB_SEARCH_IN_NEWAGE_CATEGORY         },
    {"cddb_search_in_reggae_categories",        CV_TYPE_BOOL,    &CDDB_SEARCH_IN_REGGAE_CATEGORY         },
    {"cddb_search_in_rock_categories",          CV_TYPE_BOOL,    &CDDB_SEARCH_IN_ROCK_CATEGORY           },
    {"cddb_search_in_soundtrack_categories",    CV_TYPE_BOOL,    &CDDB_SEARCH_IN_SOUNDTRACK_CATEGORY     },

    {"cddb_set_to_all_fields",                  CV_TYPE_BOOL,    &CDDB_SET_TO_ALL_FIELDS                 },
    {"cddb_set_to_title",                       CV_TYPE_BOOL,    &CDDB_SET_TO_TITLE                      },
    {"cddb_set_to_artist",                      CV_TYPE_BOOL,    &CDDB_SET_TO_ARTIST                     },
    {"cddb_set_to_album",                       CV_TYPE_BOOL,    &CDDB_SET_TO_ALBUM                      },
    {"cddb_set_to_year",                        CV_TYPE_BOOL,    &CDDB_SET_TO_YEAR                       },
    {"cddb_set_to_track",                       CV_TYPE_BOOL,    &CDDB_SET_TO_TRACK                      },
    {"cddb_set_to_track_total",                 CV_TYPE_BOOL,    &CDDB_SET_TO_TRACK_TOTAL                },
    {"cddb_set_to_genre",                       CV_TYPE_BOOL,    &CDDB_SET_TO_GENRE                      },
    {"cddb_set_to_file_name",                   CV_TYPE_BOOL,    &CDDB_SET_TO_FILE_NAME                  },

    {"cddb_run_scanner",                        CV_TYPE_BOOL,    &CDDB_RUN_SCANNER                       },

    {"set_search_window_position",              CV_TYPE_BOOL,    &SET_SEARCH_WINDOW_POSITION             },
    {"search_window_x",                         CV_TYPE_INT,     &SEARCH_WINDOW_X                        },
    {"search_window_y",                         CV_TYPE_INT,     &SEARCH_WINDOW_Y                        },
    {"search_window_height",                    CV_TYPE_INT,     &SEARCH_WINDOW_HEIGHT                   },
    {"search_window_width",                     CV_TYPE_INT,     &SEARCH_WINDOW_WIDTH                    },
    {"search_in_filename",                      CV_TYPE_BOOL,    &SEARCH_IN_FILENAME                     },
    {"search_in_tag",                           CV_TYPE_BOOL,    &SEARCH_IN_TAG                          },
    {"search_case_sensitive",                   CV_TYPE_BOOL,    &SEARCH_CASE_SENSITIVE                  },

    {"scan_tag_default_mask",                   CV_TYPE_STRING,  &SCAN_TAG_DEFAULT_MASK                  },
    {"rename_file_default_mask",                CV_TYPE_STRING,  &RENAME_FILE_DEFAULT_MASK               },
    {"rename_directory_default_mask",           CV_TYPE_STRING,  &RENAME_DIRECTORY_DEFAULT_MASK          },
    {"rename_directory_with_mask",              CV_TYPE_BOOL,    &RENAME_DIRECTORY_WITH_MASK             },

    {"options_notebook_page",                   CV_TYPE_INT,     &OPTIONS_NOTEBOOK_PAGE                  },
    {"options_window_height",                   CV_TYPE_INT,     &OPTIONS_WINDOW_HEIGHT                  },
    {"options_window_width",                    CV_TYPE_INT,     &OPTIONS_WINDOW_WIDTH                   }

};




/*************
 * Functions *
 *************/

/*
 * Define and Load default values into config variables
 */
void Init_Config_Variables (void)
{

    /*
     * Common
     */
    LOAD_ON_STARTUP               = 0;
    DEFAULT_PATH_TO_MP3           = g_strdup(HOME_VARIABLE);
    BROWSE_SUBDIR                 = 1;
#ifdef WIN32
    BROWSE_HIDDEN_DIR             = 1;
#else
    BROWSE_HIDDEN_DIR             = 0;
#endif
    OPEN_SELECTED_BROWSER_NODE    = 1;

    /*
     * Misc
     */
    SET_MAIN_WINDOW_POSITION        = 1; // Set it to '0' if problem with some Windows Manager
    MAIN_WINDOW_X                   = -1; // '-1' lets the Windows Manager to place the window
    MAIN_WINDOW_Y                   = -1;
    MAIN_WINDOW_WIDTH               = 1040;
    MAIN_WINDOW_HEIGHT              = -1;
    PANE_HANDLE_POSITION1           = 660;
    PANE_HANDLE_POSITION2           = 360;
    PANE_HANDLE_POSITION3           = 300;
    PANE_HANDLE_POSITION4           = 300;
    SHOW_HEADER_INFO                = 1;
    CHANGED_FILES_DISPLAYED_TO_RED  = 1;
    CHANGED_FILES_DISPLAYED_TO_BOLD = 0;

    DATE_AUTO_COMPLETION                    = 1;
    NUMBER_TRACK_FORMATED                   = 1;
    NUMBER_TRACK_FORMATED_SPIN_BUTTON       = 2;
    OGG_TAG_WRITE_XMMS_COMMENT              = 1;
    SET_FOCUS_TO_SAME_TAG_FIELD             = 1;
    SET_FOCUS_TO_FIRST_TAG_FIELD            = 0;
    SORTING_FILE_MODE                       = SORTING_BY_ASCENDING_FILENAME;
#ifdef WIN32
    SORTING_FILE_CASE_SENSITIVE             = 1;
#else
    SORTING_FILE_CASE_SENSITIVE             = 0;
#endif
    LOG_MAX_LINES                           = 50;
    SHOW_LOG_VIEW                           = 1;

    MESSAGE_BOX_POSITION_NONE               = 0;
    MESSAGE_BOX_POSITION_CENTER             = 0;
    MESSAGE_BOX_POSITION_MOUSE              = 0;
    MESSAGE_BOX_POSITION_CENTER_ON_PARENT   = 1;

#ifdef WIN32
    AUDIO_FILE_PLAYER                       = ET_Win32_Get_Audio_File_Player();
#else
    AUDIO_FILE_PLAYER                       = g_strdup("xmms -p");
#endif

    /*
     * File Settings
     */
    REPLACE_ILLEGAL_CHARACTERS_IN_FILENAME      = 1;
    FILENAME_EXTENSION_LOWER_CASE               = 1;
    FILENAME_EXTENSION_UPPER_CASE               = 0;
    FILENAME_EXTENSION_NO_CHANGE                = 0;
    PRESERVE_MODIFICATION_TIME                  = 1;
    UPDATE_PARENT_DIRECTORY_MODIFICATION_TIME   = 0;

    FILENAME_CHARACTER_SET_OTHER                = 1;
    FILENAME_CHARACTER_SET_APPROXIMATE          = 0;
    FILENAME_CHARACTER_SET_DISCARD              = 0;

    /*
     * Tag Settings
     */
    WRITE_ID3_TAGS_IN_FLAC_FILE                     = 0;
    STRIP_TAG_WHEN_EMPTY_FIELDS                     = 1;
    CONVERT_OLD_ID3V2_TAG_VERSION                   = 1;
    USE_NON_STANDARD_ID3_READING_CHARACTER_SET      = 0;
    FILE_READING_ID3V1V2_CHARACTER_SET              = g_strdup("UTF-8");
    FILE_WRITING_ID3V2_WRITE_TAG                    = 1;
#ifdef WIN32
    FILE_WRITING_ID3V2_VERSION_4                    = 0;
#else
    FILE_WRITING_ID3V2_VERSION_4                    = 1;
#endif
    FILE_WRITING_ID3V2_USE_CRC32                    = 0;
    FILE_WRITING_ID3V2_USE_COMPRESSION              = 0;
    FILE_WRITING_ID3V2_USE_UNICODE_CHARACTER_SET    = 1;
#ifdef WIN32
    FILE_WRITING_ID3V2_UNICODE_CHARACTER_SET        = g_strdup("UTF-16");
#else
    FILE_WRITING_ID3V2_UNICODE_CHARACTER_SET        = g_strdup("UTF-8");
#endif
    FILE_WRITING_ID3V2_NO_UNICODE_CHARACTER_SET     = g_strdup("ISO-8859-1");
    FILE_WRITING_ID3V2_ICONV_OPTIONS_NO             = 1;
    FILE_WRITING_ID3V2_ICONV_OPTIONS_TRANSLIT       = 0;
    FILE_WRITING_ID3V2_ICONV_OPTIONS_IGNORE         = 0;
    FILE_WRITING_ID3V1_WRITE_TAG                    = 1;
    FILE_WRITING_ID3V1_CHARACTER_SET                = g_strdup("ISO-8859-1");
    FILE_WRITING_ID3V1_ICONV_OPTIONS_NO             = 0;
    FILE_WRITING_ID3V1_ICONV_OPTIONS_TRANSLIT       = 1;
    FILE_WRITING_ID3V1_ICONV_OPTIONS_IGNORE         = 0;

    VORBIS_SPLIT_FIELD_TITLE                          = 1;
    VORBIS_SPLIT_FIELD_ARTIST                         = 1;
    VORBIS_SPLIT_FIELD_ALBUM                          = 1;
    VORBIS_SPLIT_FIELD_GENRE                          = 1;
    VORBIS_SPLIT_FIELD_COMMENT                        = 1;
    VORBIS_SPLIT_FIELD_COMPOSER                       = 1;
    VORBIS_SPLIT_FIELD_ORIG_ARTIST                    = 1;
    /*
     * Scanner
     */
    SCANNER_TYPE                              = SCANNER_FILL_TAG;
    SCAN_MASK_EDITOR_BUTTON                   = 0;
    SCAN_LEGEND_BUTTON                        = 0;
    FTS_CONVERT_UNDERSCORE_AND_P20_INTO_SPACE = 1;
    FTS_CONVERT_SPACE_INTO_UNDERSCORE         = 0;
    RFS_CONVERT_UNDERSCORE_AND_P20_INTO_SPACE = 1;
    RFS_CONVERT_SPACE_INTO_UNDERSCORE         = 0;
    PFS_DONT_UPPER_SOME_WORDS                 = 0;
    OVERWRITE_TAG_FIELD                       = 1;
    SET_DEFAULT_COMMENT                       = 0;
    DEFAULT_COMMENT                           = g_strdup("Tagged with EasyTAG");
    SET_CRC32_COMMENT                         = 0;
    OPEN_SCANNER_WINDOW_ON_STARTUP            = 0;
    SCANNER_WINDOW_ON_TOP                     = 1;
    SET_SCANNER_WINDOW_POSITION               = 1; // Set it to '0' if problem with some Windows Manager
    SCANNER_WINDOW_X                          = -1;
    SCANNER_WINDOW_Y                          = -1;

    /*
     * Confirmation
     */
    CONFIRM_BEFORE_EXIT    = 1;
    CONFIRM_WRITE_TAG      = 1;
    CONFIRM_RENAME_FILE    = 1;
    CONFIRM_DELETE_FILE    = 1;
    CONFIRM_WRITE_PLAYLIST = 1;

    /*
     * Scanner window
     */
    PROCESS_FILENAME_FIELD             = 0;
    PROCESS_TITLE_FIELD                = 1;
    PROCESS_ARTIST_FIELD               = 1;
    PROCESS_ALBUM_ARTIST_FIELD         = 1;
    PROCESS_ALBUM_FIELD                = 1;
    PROCESS_GENRE_FIELD                = 1;
    PROCESS_COMMENT_FIELD              = 1;
    PROCESS_COMPOSER_FIELD             = 1;
    PROCESS_ORIG_ARTIST_FIELD          = 1;
    PROCESS_COPYRIGHT_FIELD            = 1;
    PROCESS_URL_FIELD                  = 1;
    PROCESS_ENCODED_BY_FIELD           = 1;

    PROCESS_FIELDS_CONVERT_FROM        = NULL;
    PROCESS_FIELDS_CONVERT_TO          = NULL;

    PF_CONVERT_INTO_SPACE              = 1;
    PF_CONVERT_SPACE                   = 0;
    PF_CONVERT                         = 0;
    PF_CONVERT_ALL_UPPERCASE           = 0;
    PF_CONVERT_ALL_DOWNCASE            = 0;
    PF_CONVERT_FIRST_LETTER_UPPERCASE  = 0;
    PF_CONVERT_FIRST_LETTERS_UPPERCASE = 1;
    PF_DETECT_ROMAN_NUMERALS           = 1;
    PF_REMOVE_SPACE                    = 0;
    PF_INSERT_SPACE                    = 0;
    PF_ONLY_ONE_SPACE                  = 1;

    /*
     * Playlist window
     */
    PLAYLIST_NAME                   = g_strdup("playlist_%a_-_%b");
    PLAYLIST_USE_MASK_NAME          = 0;
    PLAYLIST_USE_DIR_NAME           = 1;
    PLAYLIST_ONLY_SELECTED_FILES    = 1;
    PLAYLIST_FULL_PATH              = 0;
    PLAYLIST_RELATIVE_PATH          = 1;
    PLAYLIST_CREATE_IN_PARENT_DIR   = 0;
    PLAYLIST_USE_DOS_SEPARATOR      = 0;
    PLAYLIST_CONTENT_NONE           = 0;
    PLAYLIST_CONTENT_FILENAME       = 1;
    PLAYLIST_CONTENT_MASK           = 0;
    PLAYLIST_CONTENT_MASK_VALUE     = g_strdup("%n/%l - %a - %b - %t");

    PLAYLIST_WINDOW_X               = -1;
    PLAYLIST_WINDOW_Y               = -1;
    PLAYLIST_WINDOW_WIDTH           = -1;
    PLAYLIST_WINDOW_HEIGHT          = -1;

    /*
     * Load File window
     */
    LOAD_FILE_RUN_SCANNER     = 0;
    LOAD_FILE_WINDOW_X        = -1;
    LOAD_FILE_WINDOW_Y        = -1;
    LOAD_FILE_WINDOW_WIDTH    = -1;
    LOAD_FILE_WINDOW_HEIGHT   = -1;

    /*
     * CDDB window
     */
    CDDB_SERVER_NAME_AUTOMATIC_SEARCH       = g_strdup("freedb.freedb.org");
    CDDB_SERVER_PORT_AUTOMATIC_SEARCH       = 80;
    CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH   = g_strdup("/~cddb/cddb.cgi");
    CDDB_SERVER_NAME_AUTOMATIC_SEARCH2      = g_strdup("freedb.musicbrainz.org");
    CDDB_SERVER_PORT_AUTOMATIC_SEARCH2      = 80;
    CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH2  = g_strdup("/~cddb/cddb.cgi");
    CDDB_SERVER_NAME_MANUAL_SEARCH          = g_strdup("www.gnudb.org");
    CDDB_SERVER_PORT_MANUAL_SEARCH          = 80;
    CDDB_SERVER_CGI_PATH_MANUAL_SEARCH      = g_strdup("/~cddb/cddb.cgi");
    CDDB_LOCAL_PATH                         = NULL;
    CDDB_USE_PROXY                          = 0;
    CDDB_PROXY_NAME                         = g_strdup("localhost");
    CDDB_PROXY_PORT                         = 8080;
    CDDB_PROXY_USER_NAME                    = NULL;
    CDDB_PROXY_USER_PASSWORD                = NULL;

    SET_CDDB_WINDOW_POSITION      = 1; // Set it to '0' if problem with some Windows Manager
    CDDB_WINDOW_X                 = -1;
    CDDB_WINDOW_Y                 = -1;
    CDDB_WINDOW_WIDTH             = 660;
    CDDB_WINDOW_HEIGHT            = 470;
    CDDB_PANE_HANDLE_POSITION     = 350;

    CDDB_FOLLOW_FILE              = 1;
    CDDB_USE_DLM                  = 0;
    CDDB_USE_LOCAL_ACCESS         = 0;

    CDDB_SEARCH_IN_ALL_FIELDS           = 0;
    CDDB_SEARCH_IN_ARTIST_FIELD         = 1;
    CDDB_SEARCH_IN_TITLE_FIELD          = 1;
    CDDB_SEARCH_IN_TRACK_NAME_FIELD     = 0;
    CDDB_SEARCH_IN_OTHER_FIELD          = 0;
    CDDB_SHOW_CATEGORIES                = 0;

    CDDB_SEARCH_IN_ALL_CATEGORIES       = 1;
    CDDB_SEARCH_IN_BLUES_CATEGORY       = 0;
    CDDB_SEARCH_IN_CLASSICAL_CATEGORY   = 0;
    CDDB_SEARCH_IN_COUNTRY_CATEGORY     = 0;
    CDDB_SEARCH_IN_FOLK_CATEGORY        = 0;
    CDDB_SEARCH_IN_JAZZ_CATEGORY        = 0;
    CDDB_SEARCH_IN_MISC_CATEGORY        = 1;
    CDDB_SEARCH_IN_NEWAGE_CATEGORY      = 1;
    CDDB_SEARCH_IN_REGGAE_CATEGORY      = 0;
    CDDB_SEARCH_IN_ROCK_CATEGORY        = 1;
    CDDB_SEARCH_IN_SOUNDTRACK_CATEGORY  = 0;

    CDDB_SET_TO_ALL_FIELDS  = 1;
    CDDB_SET_TO_TITLE       = 1;
    CDDB_SET_TO_ARTIST      = 0;
    CDDB_SET_TO_ALBUM       = 0;
    CDDB_SET_TO_YEAR        = 0;
    CDDB_SET_TO_TRACK       = 1;
    CDDB_SET_TO_TRACK_TOTAL = 1;
    CDDB_SET_TO_GENRE       = 0;
    CDDB_SET_TO_FILE_NAME   = 1;

    CDDB_RUN_SCANNER        = 0;

    /*
     * Search window
     */
    SET_SEARCH_WINDOW_POSITION  = 1; // Set it to '0' if problem with some Windows Manager
    SEARCH_WINDOW_X             = -1;
    SEARCH_WINDOW_Y             = -1;
    SEARCH_WINDOW_HEIGHT        = 350;
    SEARCH_WINDOW_WIDTH         = 650;
    SEARCH_IN_FILENAME          = 1;
    SEARCH_IN_TAG               = 1;
    SEARCH_CASE_SENSITIVE       = 0;

    /*
     * Masks
     */
    SCAN_TAG_DEFAULT_MASK           = NULL;
    RENAME_FILE_DEFAULT_MASK        = NULL;
    RENAME_DIRECTORY_DEFAULT_MASK   = NULL;
    RENAME_DIRECTORY_WITH_MASK      = 0;

    /*
     * Other parameters
     */
    OPTIONS_NOTEBOOK_PAGE = 0;
    OPTIONS_WINDOW_HEIGHT = 300;
    OPTIONS_WINDOW_WIDTH  = 400;

}



/*
 * Function called when pressing the "Save" button of the preferences window.
 * Save into the config variables the settings of each tab of the Preferences window...
 * If settings needs to be "shown/applied" to the corresponding window, we do it
 */
void Apply_Changes_Of_Preferences_Window (void)
{
    gchar *temp;
    int active;

    if (OptionsWindow)
    {
        /* Common */
        LOAD_ON_STARTUP               = GTK_TOGGLE_BUTTON(LoadOnStartup)->active;
        if (DEFAULT_PATH_TO_MP3) g_free(DEFAULT_PATH_TO_MP3);
        DEFAULT_PATH_TO_MP3           = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(DefaultPathToMp3)->child))); // Saved in UTF-8
//#ifdef WIN32
//        ET_Win32_Path_Replace_Backslashes(DEFAULT_PATH_TO_MP3);
//#endif
        BROWSE_SUBDIR                 = GTK_TOGGLE_BUTTON(BrowseSubdir)->active;
        BROWSE_HIDDEN_DIR             = GTK_TOGGLE_BUTTON(BrowseHiddendir)->active;
        OPEN_SELECTED_BROWSER_NODE    = GTK_TOGGLE_BUTTON(OpenSelectedBrowserNode)->active;

        /* User interface */
        SHOW_HEADER_INFO              = GTK_TOGGLE_BUTTON(ShowHeaderInfos)->active;
        // We reload the list if the selected style have changed
        if (CHANGED_FILES_DISPLAYED_TO_RED != GTK_TOGGLE_BUTTON(ChangedFilesDisplayedToRed)->active)
        {
            CHANGED_FILES_DISPLAYED_TO_RED  = GTK_TOGGLE_BUTTON(ChangedFilesDisplayedToRed)->active;
            CHANGED_FILES_DISPLAYED_TO_BOLD = GTK_TOGGLE_BUTTON(ChangedFilesDisplayedToBold)->active;
            Browser_List_Refresh_Whole_List();
        }

        /* Misc */
        DATE_AUTO_COMPLETION                   = GTK_TOGGLE_BUTTON(DateAutoCompletion)->active;
        NUMBER_TRACK_FORMATED                  = GTK_TOGGLE_BUTTON(NumberTrackFormated)->active;
        NUMBER_TRACK_FORMATED_SPIN_BUTTON      = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(NumberTrackFormatedSpinButton));
        OGG_TAG_WRITE_XMMS_COMMENT             = GTK_TOGGLE_BUTTON(OggTagWriteXmmsComment)->active;
        SORTING_FILE_CASE_SENSITIVE            = GTK_TOGGLE_BUTTON(SortingFileCaseSensitive)->active;
        SET_FOCUS_TO_SAME_TAG_FIELD            = GTK_TOGGLE_BUTTON(SetFocusToSameTagField)->active;
        SET_FOCUS_TO_FIRST_TAG_FIELD           = GTK_TOGGLE_BUTTON(SetFocusToFirstTagField)->active;
        LOG_MAX_LINES                          = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(LogMaxLinesSpinButton));
        SHOW_LOG_VIEW                          = GTK_TOGGLE_BUTTON(ShowLogView)->active;

        SORTING_FILE_MODE = gtk_combo_box_get_active(GTK_COMBO_BOX(SortingFileCombo));

        MESSAGE_BOX_POSITION_NONE               = GTK_TOGGLE_BUTTON(MessageBoxPositionNone)->active;
        MESSAGE_BOX_POSITION_CENTER             = GTK_TOGGLE_BUTTON(MessageBoxPositionCenter)->active;
        MESSAGE_BOX_POSITION_MOUSE              = GTK_TOGGLE_BUTTON(MessageBoxPositionMouse)->active;
        MESSAGE_BOX_POSITION_CENTER_ON_PARENT   = GTK_TOGGLE_BUTTON(MessageBoxPositionCenterOnParent)->active;

        if (AUDIO_FILE_PLAYER) g_free(AUDIO_FILE_PLAYER);
        AUDIO_FILE_PLAYER                       = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(FilePlayerCombo)->child)));

        /* File Settings */
        REPLACE_ILLEGAL_CHARACTERS_IN_FILENAME    = GTK_TOGGLE_BUTTON(ReplaceIllegalCharactersInFilename)->active;
        FILENAME_EXTENSION_LOWER_CASE             = GTK_TOGGLE_BUTTON(FilenameExtensionLowerCase)->active;
        FILENAME_EXTENSION_UPPER_CASE             = GTK_TOGGLE_BUTTON(FilenameExtensionUpperCase)->active;
        FILENAME_EXTENSION_NO_CHANGE              = GTK_TOGGLE_BUTTON(FilenameExtensionNoChange)->active;
        PRESERVE_MODIFICATION_TIME                = GTK_TOGGLE_BUTTON(PreserveModificationTime)->active;
        UPDATE_PARENT_DIRECTORY_MODIFICATION_TIME = GTK_TOGGLE_BUTTON(UpdateParentDirectoryModificationTime)->active;

        FILENAME_CHARACTER_SET_OTHER              = GTK_TOGGLE_BUTTON(FilenameCharacterSetOther)->active;
        FILENAME_CHARACTER_SET_APPROXIMATE        = GTK_TOGGLE_BUTTON(FilenameCharacterSetApproximate)->active;
        FILENAME_CHARACTER_SET_DISCARD            = GTK_TOGGLE_BUTTON(FilenameCharacterSetDiscard)->active;

        /* Tag Settings */
        WRITE_ID3_TAGS_IN_FLAC_FILE                = GTK_TOGGLE_BUTTON(WriteId3TagsInFlacFiles)->active;
        STRIP_TAG_WHEN_EMPTY_FIELDS                = GTK_TOGGLE_BUTTON(StripTagWhenEmptyFields)->active;
        CONVERT_OLD_ID3V2_TAG_VERSION              = GTK_TOGGLE_BUTTON(ConvertOldId3v2TagVersion)->active;
        USE_NON_STANDARD_ID3_READING_CHARACTER_SET = GTK_TOGGLE_BUTTON(UseNonStandardId3ReadingCharacterSet)->active;

#ifdef ENABLE_ID3LIB
        active = gtk_combo_box_get_active(GTK_COMBO_BOX(FileWritingId3v2VersionCombo));
        FILE_WRITING_ID3V2_VERSION_4 = !active;
#else
        FILE_WRITING_ID3V2_VERSION_4 = 1;
#endif
        temp = Get_Active_Combo_Box_Item(GTK_COMBO_BOX(FileReadingId3v1v2CharacterSetCombo));
        FILE_READING_ID3V1V2_CHARACTER_SET = Charset_Get_Name_From_Title(temp);
        g_free(temp);

        FILE_WRITING_ID3V2_WRITE_TAG                 = GTK_TOGGLE_BUTTON(FileWritingId3v2WriteTag)->active;
        FILE_WRITING_ID3V2_USE_CRC32                 = GTK_TOGGLE_BUTTON(FileWritingId3v2UseCrc32)->active;
        FILE_WRITING_ID3V2_USE_COMPRESSION           = GTK_TOGGLE_BUTTON(FileWritingId3v2UseCompression)->active;
        FILE_WRITING_ID3V2_TEXT_ONLY_GENRE           = GTK_TOGGLE_BUTTON(FileWritingId3v2TextOnlyGenre)->active;
        FILE_WRITING_ID3V2_USE_UNICODE_CHARACTER_SET = GTK_TOGGLE_BUTTON(FileWritingId3v2UseUnicodeCharacterSet)->active;

        active = gtk_combo_box_get_active(GTK_COMBO_BOX(FileWritingId3v2UnicodeCharacterSetCombo));
        FILE_WRITING_ID3V2_UNICODE_CHARACTER_SET     = (active == 1) ? "UTF-16" : "UTF-8";

        temp = Get_Active_Combo_Box_Item(GTK_COMBO_BOX(FileWritingId3v2NoUnicodeCharacterSetCombo));
        FILE_WRITING_ID3V2_NO_UNICODE_CHARACTER_SET  = Charset_Get_Name_From_Title(temp);
        g_free(temp);

        FILE_WRITING_ID3V2_ICONV_OPTIONS_NO          = GTK_TOGGLE_BUTTON(FileWritingId3v2IconvOptionsNo)->active;
        FILE_WRITING_ID3V2_ICONV_OPTIONS_TRANSLIT    = GTK_TOGGLE_BUTTON(FileWritingId3v2IconvOptionsTranslit)->active;
        FILE_WRITING_ID3V2_ICONV_OPTIONS_IGNORE      = GTK_TOGGLE_BUTTON(FileWritingId3v2IconvOptionsIgnore)->active;

        FILE_WRITING_ID3V1_WRITE_TAG                 = GTK_TOGGLE_BUTTON(FileWritingId3v1WriteTag)->active;
        temp = Get_Active_Combo_Box_Item(GTK_COMBO_BOX(FileWritingId3v1CharacterSetCombo));
        FILE_WRITING_ID3V1_CHARACTER_SET             = Charset_Get_Name_From_Title(temp);
        g_free(temp);

        FILE_WRITING_ID3V1_ICONV_OPTIONS_NO          = GTK_TOGGLE_BUTTON(FileWritingId3v1IconvOptionsNo)->active;
        FILE_WRITING_ID3V1_ICONV_OPTIONS_TRANSLIT    = GTK_TOGGLE_BUTTON(FileWritingId3v1IconvOptionsTranslit)->active;
        FILE_WRITING_ID3V1_ICONV_OPTIONS_IGNORE      = GTK_TOGGLE_BUTTON(FileWritingId3v1IconvOptionsIgnore)->active;

        VORBIS_SPLIT_FIELD_TITLE                       = GTK_TOGGLE_BUTTON(VorbisSplitFieldTitle)->active;
        VORBIS_SPLIT_FIELD_ARTIST                      = GTK_TOGGLE_BUTTON(VorbisSplitFieldArtist)->active;
        VORBIS_SPLIT_FIELD_ALBUM                       = GTK_TOGGLE_BUTTON(VorbisSplitFieldAlbum)->active;
        VORBIS_SPLIT_FIELD_GENRE                       = GTK_TOGGLE_BUTTON(VorbisSplitFieldGenre)->active;
        VORBIS_SPLIT_FIELD_COMMENT                     = GTK_TOGGLE_BUTTON(VorbisSplitFieldComment)->active;
        VORBIS_SPLIT_FIELD_COMPOSER                    = GTK_TOGGLE_BUTTON(VorbisSplitFieldComposer)->active;
        VORBIS_SPLIT_FIELD_ORIG_ARTIST                 = GTK_TOGGLE_BUTTON(VorbisSplitFieldOrigArtist)->active;

        /* Scanner */
        // Fill Tag Scanner
        FTS_CONVERT_UNDERSCORE_AND_P20_INTO_SPACE = GTK_TOGGLE_BUTTON(FTSConvertUnderscoreAndP20IntoSpace)->active;
        FTS_CONVERT_SPACE_INTO_UNDERSCORE         = GTK_TOGGLE_BUTTON(FTSConvertSpaceIntoUnderscore)->active;
        // Rename File Scanner
        RFS_CONVERT_UNDERSCORE_AND_P20_INTO_SPACE = GTK_TOGGLE_BUTTON(RFSConvertUnderscoreAndP20IntoSpace)->active;
        RFS_CONVERT_SPACE_INTO_UNDERSCORE         = GTK_TOGGLE_BUTTON(RFSConvertSpaceIntoUnderscore)->active;
        // Process File Scanner
        PFS_DONT_UPPER_SOME_WORDS                 = GTK_TOGGLE_BUTTON(PFSDontUpperSomeWords)->active;

        OVERWRITE_TAG_FIELD = GTK_TOGGLE_BUTTON(OverwriteTagField)->active;
        SET_DEFAULT_COMMENT = GTK_TOGGLE_BUTTON(SetDefaultComment)->active;
        if (DEFAULT_COMMENT) g_free(DEFAULT_COMMENT);
        DEFAULT_COMMENT     = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(DefaultComment)->child)));
        SET_CRC32_COMMENT   = GTK_TOGGLE_BUTTON(Crc32Comment)->active;

        OPEN_SCANNER_WINDOW_ON_STARTUP = GTK_TOGGLE_BUTTON(OpenScannerWindowOnStartup)->active;
        SCANNER_WINDOW_ON_TOP          = GTK_TOGGLE_BUTTON(ScannerWindowOnTop)->active;

        /* CDDB */
        if (CDDB_SERVER_NAME_AUTOMATIC_SEARCH) g_free(CDDB_SERVER_NAME_AUTOMATIC_SEARCH);
        CDDB_SERVER_NAME_AUTOMATIC_SEARCH     = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(CddbServerNameAutomaticSearch)->child)));
        CDDB_SERVER_PORT_AUTOMATIC_SEARCH     = atoi(gtk_entry_get_text(GTK_ENTRY(CddbServerPortAutomaticSearch)));
        if (CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH) g_free(CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH);
        CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH = g_strdup(gtk_entry_get_text(GTK_ENTRY(CddbServerCgiPathAutomaticSearch)));

        if (CDDB_SERVER_NAME_AUTOMATIC_SEARCH2) g_free(CDDB_SERVER_NAME_AUTOMATIC_SEARCH2);
        CDDB_SERVER_NAME_AUTOMATIC_SEARCH2     = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(CddbServerNameAutomaticSearch2)->child)));
        CDDB_SERVER_PORT_AUTOMATIC_SEARCH2     = atoi(gtk_entry_get_text(GTK_ENTRY(CddbServerPortAutomaticSearch2)));
        if (CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH2) g_free(CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH2);
        CDDB_SERVER_CGI_PATH_AUTOMATIC_SEARCH2 = g_strdup(gtk_entry_get_text(GTK_ENTRY(CddbServerCgiPathAutomaticSearch2)));

        if (CDDB_SERVER_NAME_MANUAL_SEARCH) g_free(CDDB_SERVER_NAME_MANUAL_SEARCH);
        CDDB_SERVER_NAME_MANUAL_SEARCH     = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(CddbServerNameManualSearch)->child)));
        CDDB_SERVER_PORT_MANUAL_SEARCH     = atoi(gtk_entry_get_text(GTK_ENTRY(CddbServerPortManualSearch)));
        if (CDDB_SERVER_CGI_PATH_MANUAL_SEARCH) g_free(CDDB_SERVER_CGI_PATH_MANUAL_SEARCH);
        CDDB_SERVER_CGI_PATH_MANUAL_SEARCH = g_strdup(gtk_entry_get_text(GTK_ENTRY(CddbServerCgiPathManualSearch)));

        if (CDDB_LOCAL_PATH) g_free(CDDB_LOCAL_PATH);
        CDDB_LOCAL_PATH = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_BIN(CddbLocalPath)->child)));

        CDDB_USE_PROXY       = GTK_TOGGLE_BUTTON(CddbUseProxy)->active;
        if (CDDB_PROXY_NAME) g_free(CDDB_PROXY_NAME);
        CDDB_PROXY_NAME      = g_strdup(gtk_entry_get_text(GTK_ENTRY(CddbProxyName)));
        CDDB_PROXY_PORT      = atoi(gtk_entry_get_text(GTK_ENTRY(CddbProxyPort)));
        if (CDDB_PROXY_USER_NAME) g_free(CDDB_PROXY_USER_NAME);
        CDDB_PROXY_USER_NAME          = g_strdup(gtk_entry_get_text(GTK_ENTRY(CddbProxyUserName)));
        if (CDDB_PROXY_USER_PASSWORD) g_free(CDDB_PROXY_USER_PASSWORD);
        CDDB_PROXY_USER_PASSWORD      = g_strdup(gtk_entry_get_text(GTK_ENTRY(CddbProxyUserPassword)));

        CDDB_FOLLOW_FILE      = GTK_TOGGLE_BUTTON(CddbFollowFile)->active;
        CDDB_USE_DLM          = GTK_TOGGLE_BUTTON(CddbUseDLM)->active;

        /* Confirmation */
        CONFIRM_BEFORE_EXIT    = GTK_TOGGLE_BUTTON(ConfirmBeforeExit)->active;
        CONFIRM_WRITE_TAG      = GTK_TOGGLE_BUTTON(ConfirmWriteTag)->active;
        CONFIRM_RENAME_FILE    = GTK_TOGGLE_BUTTON(ConfirmRenameFile)->active;
        CONFIRM_DELETE_FILE    = GTK_TOGGLE_BUTTON(ConfirmDeleteFile)->active;
        CONFIRM_WRITE_PLAYLIST = GTK_TOGGLE_BUTTON(ConfirmWritePlayList)->active;

        /* Parameters and variables of Scanner Window are in "scan.c" file */
        /* Parameters and variables of Cddb Window are in "cddb.c" file */
    }

    /*
     * Changes to apply to :
     */
    if (MainWindow)
    {
        if (SHOW_HEADER_INFO) gtk_widget_show_all(HeaderInfosTable);
        else                  gtk_widget_hide_all(HeaderInfosTable);

        if (SHOW_LOG_VIEW)  gtk_widget_show_all(LogArea);
        else                gtk_widget_hide_all(LogArea);

        /* Update state of check-menu-item into main/popup menu to browse subdirs */
        Check_Menu_Item_Update_Browse_Subdir();

        /* Update state of check-menu-item into main/popup menu to show hidden directories */
        Check_Menu_Item_Update_Browse_Hidden_Dir();

        /* Reload if number of character changed for track list */
        //Load_Track_List_To_UI();

        /* Reload directory, in case we have changed BROWSE_HIDDEN_DIR */
        // FIX ME : commented as it reloads files...
        //Browser_Tree_Rebuild(NULL);
    }

    if (ScannerWindow)
    {
        if (SCANNER_WINDOW_ON_TOP)
            gtk_window_set_transient_for(GTK_WINDOW(ScannerWindow),GTK_WINDOW(MainWindow));
        else
            gtk_window_set_transient_for(GTK_WINDOW(ScannerWindow),NULL);
    }

}

/*
 * Save into the config variables the settings of each window
 *  - Position/size of the window
 *  - Specific options in the window
 */
void Apply_Changes_Of_UI (void)
{
    /*
     * Changes in user interface
     */

    // Configuration of the main window (see easytag.c) - Function also called when destroying the window
    MainWindow_Apply_Changes();

    // Configuration of the preference window (see prefs.c) - Function also called when destroying the window
    OptionsWindow_Apply_Changes();

    // Configuration of the scanner window (see scan.c) - Function also called when destroying the window
    ScannerWindow_Apply_Changes();

    // Configuration of the cddb window (see cddb.c) - Function also called when destroying the window
    Cddb_Window_Apply_Changes();

    // Configuration of the playlist window (see misc.c) - Function also called when destroying the window
    Write_Playlist_Window_Apply_Changes();

    // Configuration of the search_file window (see misc.c) - Function also called when destroying the window
    Search_File_Window_Apply_Changes();

    // Configuration of the load_filename window (see misc.c) - Function also called when destroying the window
    Load_Filename_Window_Apply_Changes();

}

void Save_Changes_Of_UI (void)
{
    Apply_Changes_Of_UI();
    Save_Config_To_File();
}

void Save_Changes_Of_Preferences_Window (void)
{
    Apply_Changes_Of_Preferences_Window();
    Save_Config_To_File();

    Statusbar_Message(_("Configuration saved"),TRUE);
}



/*
 * Write the config file
 */
void Save_Config_To_File (void)
{
    gchar *file_path = NULL;
    gchar *file_path_tmp = NULL;
    FILE *file;

    /* The file to write */
    if (!HOME_VARIABLE) return;
    file_path = g_strconcat(HOME_VARIABLE,
                            HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR ? G_DIR_SEPARATOR_S : "",
                            CONFIG_FILE,NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    file_path_tmp = file_path;
    file_path = filename_from_display(file_path);
    g_free(file_path_tmp);

    if ( Create_Easytag_Directory()==0 || (file=fopen(file_path,"w+"))==0 )
    {
        Log_Print(LOG_ERROR,_("ERROR: Can't write config file: %s (%s)"),file_path,g_strerror(errno));
    }else
    {
        gint ConfigVarListLen = sizeof(Config_Variables)/sizeof(tConfigVariable);
        gint i;
        gchar *data = NULL;

        for (i=0; i<ConfigVarListLen; i++)
        {
            switch (Config_Variables[i].type)
            {
                case CV_TYPE_INT:
                {
                    data = g_strdup_printf("%s=%i\n",Config_Variables[i].name,
                                                     *(int *)Config_Variables[i].pointer);
                    fwrite(data,strlen(data),1,file);
                    //g_print("# (type:%d) %s",Config_Variables[i].type,data);
                    g_free(data);
                    break;
                }
                case CV_TYPE_BOOL:
                {
                    data = g_strdup_printf("%s=%i\n",Config_Variables[i].name,
                                                     ( *(int *)Config_Variables[i].pointer ? 1 : 0 ));
                    fwrite(data,strlen(data),1,file);
                    //g_print("# (type:%d) %s",Config_Variables[i].type,data);
                    g_free(data);
                    break;
                }
                case CV_TYPE_STRING:
                {
                    /* Doesn't write datum if empty */
                    if ( (*(char **)Config_Variables[i].pointer)==NULL ) break;

                    data = g_strdup_printf("%s=%s\n",Config_Variables[i].name,
                                                     *(char **)Config_Variables[i].pointer);
                    fwrite(data,strlen(data),1,file);
                    //g_print("# (type:%d) %s",Config_Variables[i].type,data);
                    g_free(data);
                    break;
                }
                default:
                {
                    Log_Print(LOG_ERROR,"ERROR: Can't save: type of config variable not supported "
                              "for '%s'!",Config_Variables[i].name);
                    break;
                }
            }
        }
        fclose(file);
    }
    g_free(file_path);

    //Display_Config();
}


/*
 * Parse lines read (line as <var_description>=<value>) and load the values
 * into the corresponding config variables.
 */
void Set_Config (gchar *line)
{
    gchar *var_descriptor;
    gchar *var_value;
    gint ConfigVarListLen;
    gint i;

    if (*line=='\n' || *line=='#') return;

    /* Cut string */
    var_descriptor = (gchar*)strtok(line,"=");
    var_value      = (gchar*)strtok(NULL,"=");
    //g_print("\nstr1:'%s',\t str2:'%s'",var_descriptor,var_value);

    ConfigVarListLen = sizeof(Config_Variables)/sizeof(tConfigVariable);
    for (i=0; i<ConfigVarListLen; i++)
    {
        if (Config_Variables[i].name!=NULL && var_descriptor
        && !strcmp(Config_Variables[i].name,var_descriptor))
        {
            switch (Config_Variables[i].type)
            {
                case CV_TYPE_INT:
                {
                    *(int *)Config_Variables[i].pointer = strtol(var_value, NULL, 10);
                    break;
                }

                case CV_TYPE_BOOL:
                {
                    if (strtol(var_value, NULL, 10))
                        *(int *)Config_Variables[i].pointer = 1;
                    else
                        *(int *)Config_Variables[i].pointer = 0;
                    break;
                }

                case CV_TYPE_STRING:
                {
                    if (!var_value)
                    {
                        *(char **)Config_Variables[i].pointer = NULL;
                        //g_print("\nConfig File Warning: Field of '%s' has no value!\n",var_descriptor);
                    } else
                    {
                        if ( *(char **)Config_Variables[i].pointer != NULL )
                            g_free(*(char **)Config_Variables[i].pointer);
                        *(char **)Config_Variables[i].pointer = g_malloc(strlen(var_value)+1);
                        strcpy( *(char **)Config_Variables[i].pointer,var_value );
                    }
                    break;
                }

                default:
                {
                    Log_Print(LOG_ERROR,"ERROR: Can't read: type of config variable not supported "
                              "for '%s'!",Config_Variables[i].name);
                    break;
                }
            }
        }
    }
}


/*
 * Read config from config file
 */
void Read_Config (void)
{
    gchar *file_path = NULL;
    gchar *file_path_tmp = NULL;
    FILE *file;
    gchar buffer[MAX_STRING_LEN];

    /* The file to read */
    if (!HOME_VARIABLE) return;
    file_path = g_strconcat(HOME_VARIABLE,
                            HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR?G_DIR_SEPARATOR_S:"",
                            CONFIG_FILE,NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    file_path_tmp = file_path;
    file_path = filename_from_display(file_path);
    g_free(file_path_tmp);

    if ( (file=fopen(file_path,"r"))==0 )
    {
        Log_Print(LOG_ERROR,_("Can't open configuration file '%s' (%s)"),file_path,g_strerror(errno));
        Log_Print(LOG_OK,_("Loading default configuration..."));
    }else
    {
        while (fgets(buffer,sizeof(buffer),file))
        {
            if (buffer[strlen(buffer)-1]=='\n')
                buffer[strlen(buffer)-1]='\0';
            Set_Config(buffer);
        }
        fclose(file);

        // Force this configuration! - Disabled as it is boring for russian people
        //USE_ISO_8859_1_CHARACTER_SET_TRANSLATION = 1;
        //USE_CHARACTER_SET_TRANSLATION            = 0;
    }
    g_free(file_path);
}


/*
 * Display values in config variables
 * For debuging only!
 */
void Display_Config (void)
{
    gchar *file_path = NULL;
    gchar *file_path_tmp = NULL;
    FILE *file;

    /* The file to write */
    if (!HOME_VARIABLE) return;
    file_path = g_strconcat(HOME_VARIABLE,
                            HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR?G_DIR_SEPARATOR_S:"",
                            CONFIG_FILE,NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    file_path_tmp = file_path;
    file_path = filename_from_display(file_path);
    g_free(file_path_tmp);

    if ( (file=fopen(file_path,"r"))==0 )
    {
        g_print(_("Can't open configuration file '%s' (%s)"),file_path,g_strerror(errno));
    }else
    {
        gint ConfigVarListLen = sizeof(Config_Variables)/sizeof(tConfigVariable);
        gint i;

        g_print("\n## Current Config ##");
        for (i=0; i<ConfigVarListLen; i++)
        {
            switch(Config_Variables[i].type)
            {
                case CV_TYPE_INT:
                case CV_TYPE_BOOL:
                {
                    g_print("\n%d: %s=%d",i,Config_Variables[i].name,
                                     *(int*)Config_Variables[i].pointer);
                    break;
                }
                case CV_TYPE_STRING:
                {
                    g_print("\n%d: %s=%s",i,Config_Variables[i].name,
                                     *(char**)Config_Variables[i].pointer);
                    break;
                }
                default:
                {
                    g_print("NOT IMPLEMENTED (Save_Config)!! \n\a");
                    break;
                }
            }
        }
        g_print("\n## End Current Config ##\n");
        fclose(file);
    }
    g_free(file_path);
}




/*
 * Create the main directory with empty history files
 */
gboolean Setting_Create_Files (void)
{
    gchar *home_path = NULL;
    gchar *home_path_tmp = NULL;
    gchar *file_path = NULL;
    FILE  *file;

    /* The file to write */
    if (!HOME_VARIABLE)
        return FALSE;

    if ( Create_Easytag_Directory()==FALSE )
        return FALSE;

    home_path = g_strconcat(HOME_VARIABLE,
                            HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR ? G_DIR_SEPARATOR_S : "",
                            NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    // We do it only for 'home_path' to avoid lot of code...
    home_path_tmp = home_path;
    home_path = filename_from_display(home_path);
    g_free(home_path_tmp);

    file_path = g_strconcat(home_path,CONFIG_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),CONFIG_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,SCAN_TAG_MASKS_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),SCAN_TAG_MASKS_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,RENAME_FILE_MASKS_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),RENAME_FILE_MASKS_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,RENAME_DIRECTORY_MASKS_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),RENAME_DIRECTORY_MASKS_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,DEFAULT_PATH_TO_MP3_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),DEFAULT_PATH_TO_MP3_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,DEFAULT_TAG_COMMENT_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),DEFAULT_TAG_COMMENT_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,PATH_ENTRY_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),PATH_ENTRY_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,PLAY_LIST_NAME_MASKS_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),PLAY_LIST_NAME_MASKS_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,RUN_PROGRAM_WITH_DIRECTORY_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),RUN_PROGRAM_WITH_DIRECTORY_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,RUN_PROGRAM_WITH_FILE_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),RUN_PROGRAM_WITH_FILE_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,AUDIO_FILE_PLAYER_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),AUDIO_FILE_PLAYER_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,SEARCH_FILE_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),SEARCH_FILE_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,FILE_TO_LOAD_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),FILE_TO_LOAD_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,PLAYLIST_CONTENT_MASKS_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),PLAYLIST_CONTENT_MASKS_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,CDDB_SEARCH_STRING_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),CDDB_SEARCH_STRING_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,CDDB_SEARCH_STRING_IN_RESULT_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),CDDB_SEARCH_STRING_IN_RESULT_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);

    file_path = g_strconcat(home_path,CDDB_LOCAL_PATH_HISTORY_FILE,NULL);
    if ( (file=fopen(file_path,"a+")) != NULL )
        fclose(file);
    else
        Log_Print(LOG_ERROR,_("Can't create or open file '%s' (%s)"),CDDB_LOCAL_PATH_HISTORY_FILE,g_strerror(errno));
    g_free(file_path);


    g_free(home_path);

    return TRUE;
}



/*
 * Save the contents of a list store to a file
 */
void Save_List_Store_To_File (gchar *filename, GtkListStore *liststore, gint colnum)
{
    gchar *file_path = NULL;
    gchar *file_path_tmp = NULL;
    FILE *file;
    gchar *data = NULL;
    gchar *text;
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter))
        return;

    /* The file to write */
    if (!HOME_VARIABLE) return;
    file_path = g_strconcat(HOME_VARIABLE,
                            HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR?G_DIR_SEPARATOR_S:"",
                            filename,NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    file_path_tmp = file_path;
    file_path = filename_from_display(file_path);
    g_free(file_path_tmp);

    if ( Create_Easytag_Directory()==0 || (file=fopen(file_path,"w+"))==NULL )
    {
        Log_Print(LOG_ERROR,_("ERROR: Can't write list to file: %s (%s)"),file_path,g_strerror(errno));
    }else
    {
        do
        {
            gtk_tree_model_get(GTK_TREE_MODEL(liststore), &iter, colnum, &text, -1);
            data = g_strdup_printf("%s\n",text);
            g_free(text);

            if (data)
            {
                fwrite(data,strlen(data),1,file);
                g_free(data);
            }
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore), &iter));
        fclose(file);
    }
    g_free(file_path);
}

/*
 * Populate a list store with data from a file passed in as first parameter
 */
gboolean Populate_List_Store_From_File (gchar *filename, GtkListStore *liststore, gint text_column)
{

    gchar *file_path = NULL;
    gchar *file_path_tmp = NULL;
    FILE *file;
    gchar buffer[MAX_STRING_LEN];
    GtkTreeIter iter;
    gboolean entries_set = FALSE;

    /* The file to write */
    if (!filename || !HOME_VARIABLE) return FALSE;
    file_path = g_strconcat(HOME_VARIABLE,
                            HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR?G_DIR_SEPARATOR_S:"",
                            filename,NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    file_path_tmp = file_path;
    file_path = filename_from_display(file_path);
    g_free(file_path_tmp);

    if ( (file=fopen(file_path,"r"))==NULL )
    {
        Log_Print(LOG_ERROR,_("Can't open file '%s' (%s)"),file_path,g_strerror(errno));
    }else
    {
        gchar *data = NULL;

        while(fgets(buffer,sizeof(buffer),file))
        {
            if (buffer[strlen(buffer)-1]=='\n')
                buffer[strlen(buffer)-1]='\0';

            /*if (g_utf8_validate(buffer, -1, NULL))
                data = g_strdup(buffer);
            else
                data = convert_to_utf8(buffer);*/
            data = Try_To_Validate_Utf8_String(buffer);

            if (data && g_utf8_strlen(data, -1) > 0)
            {
                gtk_list_store_append(liststore, &iter);
                gtk_list_store_set(liststore, &iter, text_column, data, -1);
                entries_set = TRUE;
            }
            g_free(data);
        }
        fclose(file);
    }
    g_free(file_path);
    return entries_set;
}


/*
 * Functions for writing and reading list of 'Fill Tag' masks
 */
void Load_Scan_Tag_Masks_List (GtkListStore *liststore, gint colnum, gchar **fallback)
{
    gint i = 0;
    GtkTreeIter iter;

    if (!Populate_List_Store_From_File(SCAN_TAG_MASKS_FILE, liststore, colnum))
    {
        // Fall back to defaults
        Log_Print(LOG_OK,_("Loading default 'Fill Tag' masks..."));

        while(fallback[i])
        {
            gtk_list_store_append(liststore, &iter);
            gtk_list_store_set(liststore, &iter, colnum, fallback[i], -1);
            i++;
        }
    }
}

void Save_Scan_Tag_Masks_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(SCAN_TAG_MASKS_FILE, liststore, colnum);
}


/*
 * Functions for writing and reading list of 'Rename File' masks
 */
void Load_Rename_File_Masks_List (GtkListStore *liststore, gint colnum, gchar **fallback)
{
    gint i = 0;
    GtkTreeIter iter;

    if (!Populate_List_Store_From_File(RENAME_FILE_MASKS_FILE, liststore, colnum))
    {
        // Fall back to defaults
        Log_Print(LOG_OK,_("Loading default 'Rename File' masks..."));

        while(fallback[i])
        {
            gtk_list_store_append(liststore, &iter);
            gtk_list_store_set(liststore, &iter, colnum, fallback[i], -1);
            i++;
        }
    }
}

void Save_Rename_File_Masks_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(RENAME_FILE_MASKS_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of 'Rename Directory' masks
 */
void Load_Rename_Directory_Masks_List (GtkListStore *liststore, gint colnum, gchar **fallback)
{
    gint i = 0;
    GtkTreeIter iter;

    if (!Populate_List_Store_From_File(RENAME_DIRECTORY_MASKS_FILE, liststore, colnum))
    {
        // Fall back to defaults
        Log_Print(LOG_OK,_("Loading default 'Rename Directory' masks..."));

        while(fallback[i])
        {
            gtk_list_store_append(liststore, &iter);
            gtk_list_store_set(liststore, &iter, colnum, fallback[i], -1);
            i++;
        }
    }
}

void Save_Rename_Directory_Masks_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(RENAME_DIRECTORY_MASKS_FILE, liststore, colnum);
}




/*
 * Functions for writing and reading list of 'DefaultPathToMp3' combobox
 */
void Load_Default_Path_To_MP3_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(DEFAULT_PATH_TO_MP3_HISTORY_FILE, liststore, colnum);
}
void Save_Default_Path_To_MP3_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(DEFAULT_PATH_TO_MP3_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of 'DefaultComment' combobox
 */
void Load_Default_Tag_Comment_Text_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(DEFAULT_TAG_COMMENT_HISTORY_FILE, liststore, colnum);
}
void Save_Default_Tag_Comment_Text_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(DEFAULT_TAG_COMMENT_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of 'BrowserEntry' combobox
 */
void Load_Path_Entry_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(PATH_ENTRY_HISTORY_FILE, liststore, colnum);
}
void Save_Path_Entry_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(PATH_ENTRY_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of 'PlayListNameCombo' combobox
 */
void Load_Play_List_Name_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(PLAY_LIST_NAME_MASKS_FILE, liststore, colnum);
}
void Save_Play_List_Name_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(PLAY_LIST_NAME_MASKS_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox to run program (tree browser)
 */
void Load_Run_Program_With_Directory_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(RUN_PROGRAM_WITH_DIRECTORY_HISTORY_FILE, liststore, colnum);
}
void Save_Run_Program_With_Directory_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(RUN_PROGRAM_WITH_DIRECTORY_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox to run program (file browser)
 */
void Load_Run_Program_With_File_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(RUN_PROGRAM_WITH_FILE_HISTORY_FILE, liststore, colnum);
}
void Save_Run_Program_With_File_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(RUN_PROGRAM_WITH_FILE_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox to run file audio player
 */
void Load_Audio_File_Player_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(AUDIO_FILE_PLAYER_HISTORY_FILE, liststore, colnum);
}
void Save_Audio_File_Player_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(AUDIO_FILE_PLAYER_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox to search a string into file (tag or filename)
 */
void Load_Search_File_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(SEARCH_FILE_HISTORY_FILE, liststore, colnum);
}
void Save_Search_File_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(SEARCH_FILE_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox of path of file to load to rename files
 */
void Load_File_To_Load_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(FILE_TO_LOAD_HISTORY_FILE, liststore, colnum);
}
void Save_File_To_Load_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(FILE_TO_LOAD_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox of playlist content
 */
void Load_Playlist_Content_Mask_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(PLAYLIST_CONTENT_MASKS_FILE, liststore, colnum);
}
void Save_Playlist_Content_Mask_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(PLAYLIST_CONTENT_MASKS_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox of cddb search string
 */
void Load_Cddb_Search_String_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(CDDB_SEARCH_STRING_HISTORY_FILE, liststore, colnum);
}
void Save_Cddb_Search_String_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(CDDB_SEARCH_STRING_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of combobox of cddb search string in result list
 */
void Load_Cddb_Search_String_In_Result_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(CDDB_SEARCH_STRING_IN_RESULT_HISTORY_FILE, liststore, colnum);
}
void Save_Cddb_Search_String_In_Result_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(CDDB_SEARCH_STRING_IN_RESULT_HISTORY_FILE, liststore, colnum);
}

/*
 * Functions for writing and reading list of 'CddbLocalPath3' combobox
 */
void Load_Cddb_Local_Path_List (GtkListStore *liststore, gint colnum)
{
    Populate_List_Store_From_File(CDDB_LOCAL_PATH_HISTORY_FILE, liststore, colnum);
}
void Save_Cddb_Local_Path_List (GtkListStore *liststore, gint colnum)
{
    Save_List_Store_To_File(CDDB_LOCAL_PATH_HISTORY_FILE, liststore, colnum);
}







/*
 * Create the directory used by EasyTAG to store files for each user.
 * If the directory already exists, does nothing and returns 1.
 * If unable to create the directory, returns 0.
 */
gboolean Create_Easytag_Directory (void)
{
    gchar *easytag_path = NULL;
    gchar *easytag_path_tmp = NULL;
    DIR *dir;

    if (!HOME_VARIABLE)
    {
        Log_Print(LOG_ERROR,_("ERROR: The environment variable HOME is not defined!"));
        return FALSE;
    }

    /* Directory to create (if doesn't exists) with absolute path
     * Note for NetBSD : avoid passing a trailing slash to mkdir() */
    easytag_path = g_strconcat(HOME_VARIABLE,
                               HOME_VARIABLE[strlen(HOME_VARIABLE)-1]!=G_DIR_SEPARATOR?G_DIR_SEPARATOR_S:"",
                               EASYTAG_DIR,
                               //EASYTAG_DIR[strlen(EASYTAG_DIR)-1]!=G_DIR_SEPARATOR?G_DIR_SEPARATOR_S:"",
                               NULL);

    // Must convert to the filesystem encoding (else may cause problem under XP with accounts like "L�o")
    easytag_path_tmp = easytag_path;
    easytag_path = filename_from_display(easytag_path);
    g_free(easytag_path_tmp);

    if ( (dir=opendir(easytag_path)) == NULL )
    {
        if ( (mkdir(easytag_path,S_IRWXU|S_IXGRP|S_IRGRP)) == -1)
        {
            Log_Print(LOG_ERROR,_("ERROR: Can't create directory '%s' (%s)!"),easytag_path,g_strerror(errno));
            return FALSE;
        }
    }else
    {
        closedir(dir);
    }

    g_free(easytag_path);

    return TRUE;
}
