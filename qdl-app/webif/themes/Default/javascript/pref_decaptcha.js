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

/* Global variables */

var accounts = [];
var decaptchaService = "";
var activeRow = -1;

function init() {
    /* Load accounts */
    
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
                            decaptchaService = JSON.parse(request.responseText).decaptchaService;
                            
                            request.onreadystatechange = function() {
                                if (request.readyState == 4) {
                                    if (request.status == 200) {
                                        accounts = JSON.parse(request.responseText);
                                        
                                        for (var i = 0; i < accounts.length; i++) {
                                            appendRow(accounts[i]);
                                        }
                                    }
                                    else {
                                        var errorString = JSON.parse(request.responseText).error;
                                        alert(errorString ? errorString : "Cannot retrieve accounts");
                                    }
                                }
                            }
                            
                            request.open("GET", "decaptchaAccounts");
                            request.send(null);
                        }
                        else {
                            var errorString = JSON.parse(request.responseText).error;
                            alert(errorString ? errorString : "Cannot retrieve accounts");
                        }
                    }
                }
            
                request.open("GET", "preferences/getProperties?properties=decaptchaService");
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

function insertRow(i, account) {
    var table = document.getElementById("accountsTable");
    
    if (i < 0) {
        i = table.rows.length;
    }
    
    var row = table.insertRow(i);
    row.setAttribute("class", "odd");
    var cell = row.insertCell(0);
    cell.setAttribute("class", "pre");
    cell.appendChild(createIcon(account.serviceIcon));
    cell.appendChild(document.createTextNode(account.serviceName));
    cell = row.insertCell(1);
    cell.appendChild(createTextEdit("usernameEdit" + (i - 1), account.username));
    cell = row.insertCell(2);
    cell.appendChild(createPasswordEdit("passwordEdit" + (i - 1), account.password));
    cell = row.insertCell(3);
    cell.appendChild(createCheckbox("checkbox" + (i - 1), account.serviceName == decaptchaService, "setDecaptchaService(" + (i - 1) + ")"));
    cell = row.insertCell(4);
    cell.appendChild(createActionButton("Save", "saveAccount(" + (i - 1) + ")"));
    cell.appendChild(createActionButton("Remove", "removeAccount(" + (i - 1) + ")"));
}

function appendRow(account) {
    insertRow(-1, account);
}

function removeRow(i) {
    document.getElementById("accountsTable").deleteRow(i);
}

function updateRow(i, account) {
    removeRow(i);
    insertRow(i, account);
}

function createIcon(icon) {
    /* Create an img element */
    
    var img = document.createElement("img");
    img.src = icon;
    img.width = "20";
    img.height = "20";
    img.align = "left";

    return img;
}

function createTextEdit(id, text) {
    /* Create an input element with type 'text' */
    
    var textEdit = document.createElement("input");
    textEdit.id = id;
    textEdit.type = "text";
    textEdit.value = text;
    
    return textEdit;
}

function createPasswordEdit(id, text) {
    /* Create an input element with type 'password' */
    
    var textEdit = document.createElement("input");
    textEdit.id = id;
    textEdit.type = "password";
    textEdit.value = text;
    
    return textEdit;
}

function createCheckbox(id, checked, callback) {
    /* Create an input element with type 'checkbox' */
    
    var checkbox = document.createElement("input");
    checkbox.id = id;
    checkbox.type = "checkbox";
    checkbox.checked = checked;
    checkbox.setAttribute("onchange", callback);
    
    return checkbox;
}

function createActionButton(text, callback) {
    /* Create an input element with type 'button' */
    
    var button = document.createElement("input");
    button.type = "button";
    button.value = text;
    button.setAttribute("onclick", callback);
    
    return button;
}

function addAccount(i) {
    var serviceName = accounts[i].serviceName;
    var username = document.getElementById("usernameEdit" + i).value;
    var password = document.getElementById("passwordEdit" + i).value;
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var account = JSON.parse(request.responseText);
                accounts.splice(i, 1, account);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot add account");
            }
        }
    }
    
    request.open("GET", "decaptchaAccounts/addAccount?serviceName=" + serviceName + "&username=" + username + "&password=" + password);
    request.send(null);
}

function removeAccount(i) {
    var serviceName = accounts[i].serviceName;
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                removeRow(i + 1);
                accounts.splice(i, 1);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot remove account");
            }
        }
    }
    
    request.open("GET", "decaptchaAccounts/removeAccount?serviceName=" + serviceName);
    request.send(null);
}

function setDecaptchaService(i) {
    var service = document.getElementById("checkbox" + i).checked ? accounts[i].serviceName : "";
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                document.getElementById("checkbox" + activeRow).checked = false;
                decaptchaService = service;
                activeRow = i;
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot set decaptcha service");
            }
        }
    }
    
    request.open("GET", "preferences/setProperties?decaptchaService=" + service);
    request.send(null);
}