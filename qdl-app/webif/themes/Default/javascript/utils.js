/*
 * Copyright (C) 2014 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

function formatBytes(bytes) {
    /* Format bytes as a string */
    
    var kb = 1024;
    var mb = kb * 1024;
    var gb = mb * 1024;

    if (bytes > gb) {
        return (parseFloat(bytes) / gb).toFixed(2) + "GB";
    }

    if (bytes > mb) {
        return (parseFloat(bytes) / mb).toFixed(2) + "MB";
    }

    if (bytes > kb) {
        return (parseFloat(bytes) / kb).toFixed(2) + "kB";
    }

    return bytes + "B";
}

function formatMSecs(msecs) {
    /* Format milliseconds to a string in "mins:secs" format */

    var date = new Date(msecs);
    var mins = date.getMinutes();
    var secs = date.getSeconds();

    if (mins < 10) {
	    mins = "0" + mins;
    }

    if (secs < 10) {
	    secs = "0" + secs;
    }

    return mins + ":" + secs;
}

function formatSecs(secs) {
    /* Format seconds to a string in "mins:secs" format */

    return formatMSecs(secs * 1000);
}
