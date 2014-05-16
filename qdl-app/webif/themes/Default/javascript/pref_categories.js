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

var categories = [];

function init() {
    /* Connect handlers */
    
    document.getElementById("nameEdit").onchange = function() {
        document.getElementById("addCategoryButton").disabled = ((this.value == "") || (document.getElementById("pathEdit").value == ""));
    }
    
    document.getElementById("nameEdit").onkeyup = function() {
        document.getElementById("addCategoryButton").disabled = ((this.value == "") || (document.getElementById("pathEdit").value == ""));
    }
    
    document.getElementById("pathEdit").onchange = function() {
        document.getElementById("addCategoryButton").disabled = ((this.value == "") || (document.getElementById("nameEdit").value == ""));
    }
    
    document.getElementById("pathEdit").onkeyup = function() {
        document.getElementById("addCategoryButton").disabled = ((this.value == "") || (document.getElementById("nameEdit").value == ""));
    }
    
    /* Load categories */
    
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
                            categories = JSON.parse(request.responseText);
                            
                            for (var i = 0; i < categories.length; i++) {
                                appendRow(categories[i]);
                            }
                        }
                        else {
                            var errorString = JSON.parse(request.responseText).error;
                            alert(errorString ? errorString : "Cannot retrieve categories");
                        }
                    }
                }
                
                request.open("GET", "categories");
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

function insertRow(i, category) {
    var table = document.getElementById("categoriesTable");
    
    if (i < 0) {
        i = table.rows.length;
    }
    
    var row = table.insertRow(i);
    row.setAttribute("class", "odd");
    var cell = row.insertCell(0);
    cell.appendChild(createTextEdit("nameEdit" + (i - 1), category.name));
    cell = row.insertCell(1);
    cell.appendChild(createTextEdit("pathEdit" + (i - 1), category.path));
    cell = row.insertCell(2);
    cell.appendChild(createActionButton("Save", "editCategory(" + (i - 1) + ")"));
    cell.appendChild(createActionButton("Remove", "removeCategory(" + (i - 1) + ")"));
}

function appendRow(category) {
    insertRow(-1, category);
}

function removeRow(i) {
    document.getElementById("categoriesTable").deleteRow(i);
}

function updateRow(i, category) {
    removeRow(i);
    insertRow(i, category);
}

function createTextEdit(id, text) {
    var textEdit = document.createElement("input");
    textEdit.id = id;
    textEdit.type = "text";
    textEdit.value = text;
    
    return textEdit;
}

function createActionButton(text, callback) {
    var button = document.createElement("input");
    button.type = "button";
    button.value = text;
    button.setAttribute("onclick", callback);
    
    return button;
}

function addCategory() {
    var name = document.getElementById("nameEdit").value;
    var path = document.getElementById("pathEdit").value;
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var category = JSON.parse(request.responseText);
                appendRow(category);
                categories.push(category);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot add category");
            }
        }
    }
    
    request.open("GET", "categories/addCategory?name=" + name + "&path=" + path);
    request.send(null);
}

function editCategory(i) {
    var name = document.getElementById("nameEdit" + i).value;
    var path = document.getElementById("pathEdit" + i).value;
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var category = JSON.parse(request.responseText);
                categories.splice(i, 1, category);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot edit category");
            }
        }
    }
    
    request.open("GET", "categories/addCategory?name=" + name + "&path=" + path);
    request.send(null);
}

function removeCategory(i) {
    var name = categories[i].name;
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                removeRow(i + 1);
                categories.splice(i, 1);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot remove category");
            }
        }
    }
    
    request.open("GET", "categories/removeCategory?name=" + name);
    request.send(null);
}