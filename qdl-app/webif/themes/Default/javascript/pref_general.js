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

function init() {
    /* Load preferences */
    
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var status = JSON.parse(request.responseText);
                totalTransfers = status.totalTransfers;
                activeTransfers = status.activeTransfers;
                downloadSpeed = status.downloadSpeed;
                
                document.getElementById("summary").innerHTML = "<b>Total downloads: " + status.totalTransfers + " </b> - <b>Active downloads: " + status.activeTransfers + "</b> - <b>Download speed: " + status.downloadSpeed + " kB/s</b>";
                document.getElementById("versionNumber").innerHTML = status.versionNumber;
                document.getElementById("downloadPathEdit").disabled = (status.totalTransfers > 0);
    
                request.onreadystatechange = function() {
                    if (request.readyState == 4) {
                        if (request.status == 200) {
                            var preferences = JSON.parse(request.responseText);
                            document.getElementById("downloadPathEdit").value = preferences.downloadPath;
                            document.getElementById("autoCheckbox").checked = preferences.startTransfersAutomatically;
                            document.getElementById("clipboardCheckbox").checked = preferences.monitorClipboard;
                            document.getElementById("extractArchivesCheckbox").checked = preferences.extractDownloadedArchives;
                            document.getElementById("archiveSubfoldersCheckbox").checked = preferences.createSubfolderForArchives;
                            document.getElementById("deleteArchivesCheckbox").checked = preferences.deleteExtractedArchives;
                        }
                        else {
                            var errorString = JSON.parse(request.responseText).error;
                            alert(errorString ? errorString : "Cannot retrieve preferences");
                        }
                    }
                }
                
                request.open("GET", "preferences/getProperties?properties=downloadPath,startTransfersAutomatically,monitorClipboard,extractDownloadedArchives,createSubfolderForArchives,deleteExtractedArchives");
                request.send(null);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot retrieve status");
            }
        }
    }
    
    request.open("GET", "status");
    request.send(null);
}

function save() {
    /* Save preferences */
    
    var downloadPath = document.getElementById("downloadPathEdit").value;
    var startTransfersAutomatically = document.getElementById("autoCheckbox").checked ? "true" : "false";
    var monitorClipboard = document.getElementById("clipboardCheckbox").checked ? "true" : "false";
    var extractDownloadedArchives = document.getElementById("extractArchivesCheckbox").checked ? "true" : "false";
    var createSubfolderForArchives = document.getElementById("archiveSubfoldersCheckbox").checked ? "true" : "false";
    var deleteExtractedArchives = document.getElementById("deleteArchivesCheckbox").checked ? "true" : "false";
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status != 200) {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot save preferences");
            }
        }
    }
    
    request.open("GET", "preferences/setProperties?downloadPath=" + downloadPath 
                + "&startTransfersAutomatically=" + startTransfersAutomatically
                + "&monitorClipboard=" + monitorClipboard
                + "&extractDownloadedArchives=" + extractDownloadedArchives
                + "&createSubfolderForArchives=" + createSubfolderForArchives
                + "&deleteExtractedArchives=" + deleteExtractedArchives);
    request.send(null);
}