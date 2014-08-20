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
    /* Connect handlers */
    
    document.getElementById("proxyCheckbox").onchange = function() {
        var proxyTypeSelector = document.getElementById("proxyTypeSelector");
        var hostEdit = document.getElementById("hostEdit");
        var portEdit = document.getElementById("portEdit");
        var usernameEdit = document.getElementById("usernameEdit");
        var passwordEdit = document.getElementById("passwordEdit");
        proxyTypeSelector.disabled = !this.checked;
        hostEdit.disabled = !this.checked;
        portEdit.disabled = !this.checked;
        usernameEdit.disabled = !this.checked;
        passwordEdit.disabled = !this.checked;
        
        if (!this.checked) {
            proxyTypeSelector.selectedIndex = 0;
            hostEdit.value = "";
            portEdit.value = "80";
            usernameEdit.value = "";
            passwordEdit.value = "";
        }
    }
    
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
    
                request.onreadystatechange = function() {
                    if (request.readyState == 4) {
                        if (request.status == 200) {
                            var preferences = JSON.parse(request.responseText);
                            document.getElementById("proxyCheckbox").checked = preferences.networkProxyHostName != "";
                            document.getElementById("hostEdit").value = preferences.networkProxyHostName;
                            document.getElementById("portEdit").value = preferences.nextworkProxyPort;
                            document.getElementById("usernameEdit").value = preferences.nextworkProxyUser;
                            document.getElementById("passwordEdit").value = preferences.nextworkProxyPassword;
                            var selector = document.getElementById("proxyTypeSelector");
                            
                            for (var i = 0; i < selector.options.length; i++) {
                                if (selector.options[i].value == preferences.networkProxyType) {
                                    selector.options[i].selected = true;
                                }
                            }
                        }
                        else {
                            var errorString = JSON.parse(request.responseText).error;
                            alert(errorString ? errorString : "Cannot retrieve preferences");
                        }
                    }
                }
                
                request.open("GET", "preferences/getProperties?properties=networkProxyHostName,networkProxyPort,networkProxyUser,networkProxyPassword,networkProxyType");
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
    
    var host = document.getElementById("hostEdit").value;
    var port = document.getElementById("portEdit").value;
    var username = document.getElementById("usernameEdit").value;
    var password = document.getElementById("passwordEdit").value;
    var selector = document.getElementById("proxyTypeSelector");
    var proxyType = selector.options[selector.selectedIndex].value;
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status != 200) {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot save preferences");
            }
        }
    }
    
    request.open("GET", "preferences/setProperties?networkProxyHost=" + host 
                + "&networkProxyPort=" + port
                + "&networkProxyUser=" + username
                + "&networkProxyPassword=" + password
                + "&networkProxyType=" + proxyType);
    request.send(null);
}
