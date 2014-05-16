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

function quitQDL() {
    /* Quit the QDL application */
    
    if (confirm("Do you wish to quit QDL?")) {
        var request = new XMLHttpRequest();

        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                if (request.status == 200) {
                    document.getElementsByTagName("body")[0].innerHTML = '<img src="images/logo.png" width="48" height="48" /><span style="font-size: 64px">QDL</span>1.0.5<br><br><h1>You have quit QDL</h1>';
                }
                else {
                    var errorString = JSON.parse(request.responseText).error;
                    alert(errorString ? errorString : "Cannot quit QDL");
                }
            }
        }

        request.open("GET", "transfers/quit");
        request.send(null);
    }
}