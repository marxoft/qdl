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

var totalTransfers = 0;
var activeTransfers = 0;
var downloadSpeed = 0;
var maximumConcurrentTransfers = 1;
var maximumConnectionsPerTransfer = 1;
var downloadRateLimit = 0;
var nextAction = 0;
var transfers = [];
var categories = [];
var priorities = [ "High", "Normal", "Low" ];

var currentIndex = document.URL.indexOf("start=") > 0 ? parseInt(document.URL.split("start=")[1].split("&")[0]) : 0;
var limitPerPage = document.URL.indexOf("limit=") > 0 ? parseInt(document.URL.split("limit=")[1].split("&")[0]) : 30;
var filter = document.URL.indexOf("filter=") > 0 ? document.URL.split("filter=")[1].split("&")[0] : "";
var query = document.URL.indexOf("query=") > 0 ? document.URL.split("query=")[1].split("&")[0] : "";

var captchaId = "";
var captchaFileName = "";
var captchaTimeOut = 0;

function init() {
    /* Connect handlers */
    
    document.getElementById("addUrlsEdit").onkeyup = function() {
        document.getElementById("addUrlsButton").disabled = (this.value == "");
    }
    
    document.getElementById("addUrlsEdit").onchange = function() {
        document.getElementById("addUrlsButton").disabled = (this.value == "");
    }
    
    document.getElementById("retrieveUrlsEdit").onkeyup = function() {
        document.getElementById("retrieveUrlsButton").disabled = (this.value == "");
    }
    
    document.getElementById("retrieveUrlsEdit").onchange = function() {
        document.getElementById("retrieveUrlsButton").disabled = (this.value == "");
    }

    document.getElementById("captchaResponseEdit").onkeyup = function() {
        document.getElementById("submitCaptchaButton").disabled = (this.value == "");
    }

    document.getElementById("captchaResponseEdit").onchange = function() {
        document.getElementById("submitCaptchaButton").disabled = (this.value == "");
    }
    
    /* Load data */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var status = JSON.parse(request.responseText);
                totalTransfers = status.totalTransfers;
                activeTransfers = status.activeTransfers;
                downloadSpeed = status.downloadSpeed;
                maximumConcurrentTransfers = status.maximumConcurrentTransfers;
                maximumConnectionsPerTransfer = status.maximumConnectionsPerTransfer;
                downloadRateLimit = status.downloadRateLimit;
                nextAction = status.nextAction;
                
                document.getElementById("summary").innerHTML = "<b>Total downloads: " + totalTransfers + " </b> - <b>Active downloads: " + activeTransfers + "</b> - <b>Download speed: " + downloadSpeed + " kB/s</b>";
                document.getElementById("versionNumber").innerHTML = status.versionNumber;
                document.getElementById("searchEdit").value = query;
                
                updateSelector(document.getElementById("concurrentSelector"), maximumConcurrentTransfers);
                updateSelector(document.getElementById("connectionsSelector"), maximumConnectionsPerTransfer);
                updateSelector(document.getElementById("rateLimitSelector"), downloadRateLimit);
                updateSelector(document.getElementById("actionSelector"), nextAction);
                updateSelector(document.getElementById("filterSelector"), filter);

                request.onreadystatechange = function() {
                    if (request.readyState == 4) {
                        if (request.status == 200) {
                            categories = JSON.parse(request.responseText);
                            
                            var selector = document.getElementById("urlsCategorySelector");
    
                            for (var i = 0; i < categories.length; i++) {
                                var option = document.createElement("option");
                                option.value = categories[i];
                                option.appendChild(document.createTextNode(categories[i]));
                                
                                if (categories[i] == "Default") {
                                    option.selected = true;        
                                }
                                
                                selector.appendChild(option);
                            }
                            
                            request.onreadystatechange = function() {
                                if (request.readyState == 4) {
                                    if (request.status == 200) {
                                        transfers = JSON.parse(request.responseText).transfers;
                                        
                                        for (var i = 0; i < transfers.length; i++) {
                                            var transfer = transfers[i];
                                            appendRow(transfer);

                                            if ((transfer.status == 8) && (!captchaId)) {
                                                captchaId = transfer.id;
                                                captchaFileName = transfer.captchaFileName;
                                                captchaTimeOut = transfer.captchaTimeOut;
                                            }
                                        }

                                        if (captchaId) {
                                            showCaptchaDialog();
                                        }
                                    }
                                }
                            }
                            
                            request.open("GET", "transfers?filter=" + filter + "&query=" + query + "&start=" + currentIndex + "&limit=" + limitPerPage);
                            request.send(null);
                        }
                    }
                }
                
                request.open("GET", "categoryNames");
                request.send(null);
            }
        }
    }

    request.open("GET", "status");
    request.send(null);
}

function showCaptchaDialog() {
    /* Display the captcha dialog enabling the user to submit a captcha response */

    document.getElementById("dialogBackground").style.display = "block";
    document.getElementById("captchaDialog").style.display = "block";
    document.getElementById("captchaImage").src = captchaFileName;
    document.getElementById("captchaTimeOut").innerHTML = formatSecs(captchaTimeOut);
    window.setTimeout(updateCaptchaDialog, 1000);
}

function updateCaptchaDialog() {
    /* Update the captcha timeout value in the captcha dialog */

    captchaTimeOut--;

    if (captchaTimeOut > 0) {
        document.getElementById("captchaTimeOut").innerHTML = formatSecs(captchaTimeOut);
        window.setTimeout(updateCaptchaDialog, 1000);
    }
    else {
        hideCaptchaDialog();
    }
}

function hideCaptchaDialog() {
    document.getElementById("captchaDialog").style.display = "none";
    document.getElementById("dialogBackground").style.display = "none";
    document.getElementById("captchaResponseEdit").value = "";
}

function submitCaptcha() {
    /* Submit the captcha response */

    var response = document.getElementById("captchaResponseEdit").value;
    setTransferProperty(captchaId, "captchaResponse", response);
    hideCaptchaDialog();
}

function cancelCaptcha() {
    /* Cancel the captcha submission by submitting an empty string as the response */

    setTransferProperty(captchaId, "captchaResponse", "");
    hideCaptchaDialog();
}

function showAddUrlsDialog(urls) {
    /* Display the dialog to add URLs */
   
    document.getElementById("dialogBackground").style.display = "block";
    document.getElementById("addUrlsDialog").style.display = "block";
    var edit = document.getElementById("addUrlsEdit");
    edit.select();
    
    if (urls) {
        edit.value = urls;
    }
}

function hideAddUrlsDialog() {
    document.getElementById("addUrlsDialog").style.display = "none";
    document.getElementById("dialogBackground").style.display = "none";
    document.getElementById("addUrlsEdit").value = "";
}

function showRetrieveUrlsDialog() {
    /* Display the dialog to retrieve URLs */
    
    document.getElementById("dialogBackground").style.display = "block";
    document.getElementById("retrieveUrlsDialog").style.display = "block";
    document.getElementById("retrieveUrlsEdit").select();
}

function hideRetrieveUrlsDialog() {
    document.getElementById("retrieveUrlsDialog").style.display = "none";
    document.getElementById("dialogBackground").style.display = "none";
    document.getElementById("retrieveUrlsEdit").value = "";
}

function showCheckUrlsDialog(urls) {
    /* Display the dialog to check URLs */
    
    var table = document.getElementById("checkUrlsTable");
    
    for (var i = 0; i < urls.length; i++) {
        var row = table.insertRow(table.rows.length);
        var cell = row.insertCell(0);
        cell.appendChild(document.createTextNode(urls[i].url));
        cell = row.insertCell(1);
        cell.appendChild(document.createTextNode(urls[i].checked ? urls[i].ok ? "Yes" : "No" : "?"));
    }
    
    document.getElementById("dialogBackground").style.display = "block";
    document.getElementById("checkUrlsDialog").style.display = "block";
    window.setTimeout(updateCheckUrlsDialog, 2000);
}

function hideCheckUrlsDialog() {
    document.getElementById("checkUrlsDialog").style.display = "none";
    document.getElementById("dialogBackground").style.display = "none";
    
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                document.getElementById("checkUrlsLabel").innerHTML = "<br><i>Checking URLs</i><br>";
                document.getElementById("cancelUrlChecksButton").disabled = false;
                document.getElementById("closeUrlChecksButton").disabled = true;
                var bar = document.getElementById("checkUrlsProgressBarFill");
                bar.style.width = "0%";
                var table = document.getElementById("checkUrlsTable");
    
                while (table.rows.length > 1) {
                    table.deleteRow(table.rows.length - 1);
                }
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot clear checked URLs");
            }
        }
    }
    
    request.open("GET", "urls/clearUrlChecks");
    request.send(null);
}

function updateCheckUrlsDialog() {
    var request = new XMLHttpRequest();
    
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var response = JSON.parse(request.responseText);
                var progress = response.progress;
                var urls = response.urls;
                
                var bar = document.getElementById("checkUrlsProgressBarFill");
                var cancelButton = document.getElementById("cancelUrlChecksButton");
                var okButton = document.getElementById("closeUrlChecksButton");
                var label = document.getElementById("checkUrlsLabel");
                var table = document.getElementById("checkUrlsTable");
                var cancelled = cancelButton.disabled;
                var completed = progress == 100;
                
                bar.style.width = progress + "%";
                cancelButton.disabled = ((completed) || (cancelled));
                okButton.disabled = !cancelButton.disabled;
                label.innerHTML = "<br><i>" + (completed ? "Completed" : cancelled ? "Cancelled" : "Checking URLs") + "</i><br>";
                
                for (var i = 0; i < urls.length; i++) {
                    var row = table.rows[i + 1];
                    
                    if (!row) {
                        row = table.insertRow(table.rows.length);
                        var cell = table.insertCell(0);
                        cell.appendChild(document.createTextNode(urls[i].url));
                        cell = row.insertCell(1);
                        cell.appendChild(document.createTextNode(urls[i].checked ? urls[i].ok ? "Yes" : "No" : "?"));
                    }
                    else {
                        row.cells[1].innerHTML = urls[i].checked ? urls[i].ok ? "Yes" : "No" : "?";
                    }
                }
                
                if ((!cancelled) && (!completed)) {
                    window.setTimeout(updateCheckUrlsDialog, 2000);
                }       
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Error updating progress");
            }
        }
    }
    
    request.open("GET", "urls/urlCheckProgress");
    request.send(null);
}

function showProgressDialog(message) {
    document.getElementById("progressMessage").innerHTML = message;
    document.getElementById("dialogBackground").style.display = "block";
    document.getElementById("progressDialog").style.display = "block";
}

function updateProgressDialog(progress) {
    var bar = document.getElementById("progressBarFill");
    bar.style.width = progress + "%";
}

function hideProgressDialog() {
    document.getElementById("progressDialog").style.display = "none";
    document.getElementById("dialogBackground").style.display = "none";
    var bar = document.getElementById("progressBarFill");
    bar.style.width = "0%";
}

function addUrls() {
    /* Add URLs to the queue */
    
    var urls = document.getElementById("addUrlsEdit").value;
    var selector = document.getElementById("urlsCategorySelector");
    var category = selector.options[selector.selectedIndex].value;
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                hideAddUrlsDialog();
                showCheckUrlsDialog(JSON.parse(request.responseText).urls);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot add URLs");
            }
        }
    }

    request.open("GET", "urls/addUrls?urls=" + encodeURIComponent(urls.replace(/\s+/g, ",")) + "&category=" + category);
    request.send(null);
}

function cancelUrlChecks() {
    /* Cancel all ongoing URL checks */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                document.getElementById("checkUrlsLabel").innerHTML = "<br><i>Cancelling</i><br>";
                document.getElementById("cancelUrlChecksButton").disabled = true;
                document.getElementById("closeUrlChecksButton").disabled = false;
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot cancel URL checks");
            }
        }
    }

    request.open("GET", "urls/cancelUrlChecks");
    request.send(null);
}

function retrieveUrls(links) {
    /* Retrieve URLs from the provided links */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                hideRetrieveUrlsDialog();
                showProgressDialog("Retrieving URLs");
                window.setTimeout(updateUrlRetrieval, 2000);
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot retrieve URLs");
            }
        }
    }

    request.open("GET", "urls/retrieveUrls?urls=" + encodeURIComponent(links.replace(/\s+/g, ",")));
    request.send(null);
}

function updateUrlRetrieval() {
    /* Check progress of ongoing URL retrieval */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                var response = JSON.parse(request.responseText);
                updateProgressDialog(response.progress);
                
                if (response.progress == 100) {
                    hideProgressDialog();
                    
                    if (response.urls.length > 0) {
                        showAddUrlsDialog(response.urls.join("\n"));
                        
                        request.onreadystatechange = function() {
                            if (request.readyState == 4) {
                                if (request.status != 200) {
                                    var errorString = JSON.parse(request.responseText).error;
                                    alert(errorString ? errorString : "Cannot clear retrieved URLs");
                                }
                            }
                        }
                        
                        request.open("GET", "urls/clearRetrievedUrls");
                        request.send(null);
                    }
                    else {
                        alert("No supported URLs found");
                    }
                }
                else {
                    window.setTimeout(updateUrlRetrieval, 2000);
                }
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot update URL retrieval progress");
            }
        }
    }

    request.open("GET", "urls/urlRetrievalProgress");
    request.send(null);
}

function cancelUrlRetrieval() {
    /* Cancel ongoing URL retrieval */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                hideProgressDialog();
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot cancel URL retrieval");
            }
        }
    }

    request.open("GET", "urls/cancelUrlRetrieval");
    request.send(null);
}

function firstPage() {
    /* Show the first page of transfers */
    
    document.location = "queue?filter=" + filter + "&query=" + query + "&start=0&limit=" + limitPerPage;
}

function lastPage() {
    /* Show the last page of transfers */
    
    if (totalTransfers > limitPerPage) {
        document.location = "queue?filter=" + filter + "&query=" + query + "&start=" + (totalTransfers - limitPerPage) + "&limit=" + limitPerPage;
    }
    else {
        firstPage();    
    }
}

function previousPage() {
    /* Show the previous page of transfers */
    
    if (currentIndex > limitPerPage) {
        document.location = "queue?filter=" + filter + "&query=" + query + "&start=" + (currentIndex - limitPerPage) + "&limit=" + limitPerPage;
    }
    else {
        firstPage();    
    }
}

function nextPage() {
    /* Show the next page of transfers */
    
    if ((currentIndex + transfers.length) < totalTransfers) {
        document.location = "queue?filter=" + filter + "&query=" + query + "&start=" + (currentIndex + limitPerPage) + "&limit=" + limitPerPage;
    }
    else {
        lastPage();    
    }
}

function filterTransfers() {
    /* Filter the transfers by status */
    
    var selector = document.getElementById("filterSelector");
    var f = selector.options[selector.selectedIndex].value;
    document.location = "queue?filter=" + f + "&query=" + query + "&start=0&limit=" + limitPerPage;
}

function searchTransfers() {
    /* Filter the transfers by name */
    
    var q = document.getElementById("searchEdit").value;
    document.location = "queue?filter=" + filter + "&query=" + q + "&start=0&limit=" + limitPerPage;
}

function updateSelector(selector, value) {
    /* Update the selected index of the selector */
    
    for (var i = 0; i < selector.length; i++) {
        if (selector.options[i].value == value) {
            selector.selectedIndex = i;
            return;
        }
    }
}

function insertRow(i, transfer) {
    /* Insert a new row in the queue table with the transfer details */
    
    var table = document.getElementById("queueTable");
    var row = table.insertRow(i < 0 ? table.rows.length : i);
    row.id = transfer.id;
    row.setAttribute("class", "odd");
    
    var cell = row.insertCell(0);
    cell.setAttribute("class", "pre");
    cell.appendChild(createIcon(transfer.icon));
    cell.appendChild(document.createTextNode(transfer.name));
    
    cell = row.insertCell(1);
    cell.appendChild(createCategorySelector(transfer.category, "setTransferProperty(" + transfer.id + ", 'category', this.options[this.selectedIndex].value)"));

    cell = row.insertCell(2);
    cell.appendChild(createConnectionsSelector(transfer.preferredConnections, transfer.maximumConnections, "setTransferProperty(" + transfer.id + ", 'preferredConnections', this.options[this.selectedIndex].value)"));

    cell = row.insertCell(3);
    cell.appendChild(createPrioritySelector(transfer.priority, "setTransferProperty(" + transfer.id + ", 'priority', this.options[this.selectedIndex].value)"));

    cell = row.insertCell(4);
    cell.setAttribute("class", "pre");
    cell.appendChild(document.createTextNode(formatBytes(transfer.position) + " of " + (!transfer.size ? "Unknown" : formatBytes(transfer.size)) + " (" + transfer.progress + "%)"));

    cell = row.insertCell(5);
    cell.setAttribute("class", "pre");
    cell.appendChild(document.createTextNode(transfer.statusString));

    cell = row.insertCell(6);
    
    switch (transfer.status) {
    case 0:
    case 3:
        cell.appendChild(createActionButton("Start", "startTransfer(" + transfer.id + ")"));
        break;
    default:
        cell.appendChild(createActionButton("Pause", "pauseTransfer(" + transfer.id + ")"));
        break;
    }
    
    cell.appendChild(createActionButton("Remove", "removeTransfer(" + transfer.id + ")"));
}

function appendRow(transfer) {
    /* Append a new row to the queue table with the transfer details */
    
    insertRow(-1, transfer);
}

function removeRow(i) {
    /* Remove the specified row from the queue table */
    
    document.getElementById("queueTable").deleteRow(i);
}

function removeRow(id) {
    /* Remove the row with the specified id from the queue table */
    
    var table = document.getElementById("queueTable");
    
    for (var i = 0; i < table.rows.length; i++) {
        if (table.rows[i].id == id) {
            table.deleteRow(i);
            return;
        }
    }
}

function updateRow(transfer) {
    /* Find the row in the queue table that represents the transfer and replace it with the new data */
    
    var table = document.getElementById("queueTable");
    
    for (var i = 0; i < table.rows.length; i++) {
        if (table.rows[i].id == transfer.id) {
            table.deleteRow(i);
            insertRow(i, transfer);
            return;
        }
    }
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

function createCategorySelector(category, callback) {
    /* Create a select element for transfer categories */
    
    var selector = document.createElement("select");
    
    for (var i = 0; i < categories.length; i++) {
        var option = document.createElement("option");
        option.value = categories[i];
        option.appendChild(document.createTextNode(categories[i]));
        
        if (categories[i] == category) {
            option.selected = true;        
        }
        
        selector.appendChild(option);
    }
   
    selector.setAttribute("onchange", callback);

    return selector;
}

function createConnectionsSelector(preferred, maximum, callback) {
    /* Create a select element for transfer connections */
    
    var selector = document.createElement("select");

    if (!preferred) {
        preferred = 1;
    }

    if (!maximum) {
        maximum = 1;
    }
    
    for (var i = 1; i <= maximum; i++) {
        var option = document.createElement("option");
        option.value = i;
        option.appendChild(document.createTextNode(i));
        
        if (i == preferred) {
            option.selected = true;        
        }
        
        selector.appendChild(option);
    }
    
    selector.setAttribute("onchange", callback);

    return selector;
}

function createPrioritySelector(priority, callback) {
    /* Create a select element for transfer priority */
    
    var selector = document.createElement("select");
    
    for (var i = 0; i < priorities.length; i++) {
        var option = document.createElement("option");
        option.value = i;
        option.appendChild(document.createTextNode(priorities[i]));
        
        if (i == priority) {
            option.selected = true;        
        }
        
        selector.appendChild(option);
    }
    
    selector.setAttribute("onchange", callback);

    return selector;
}

function createActionButton(text, callback) {
    /* Create a input button element to pause/start/remove a transfer */
    
    var button = document.createElement("input");
    button.type = "button"
    button.value = text;
    button.setAttribute("onclick", callback);
    
    return button;
}

function setTransferProperty(id, property, value) {
    /* Set the property of the transfer with the specified id to value */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                updateRow(JSON.parse(request.responseText));
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot set download property");
            }
        }
    }

    request.open("GET", "transfers/setTransferProperty?id=" + id + "&property=" + property + "&value=" + value);
    request.send(null);
}

function pauseTransfer(id) {
    /* Pause the transfer with the specified id */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                updateRow(JSON.parse(request.responseText));
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot pause download");
            }
        }
    }

    request.open("GET", "transfers/pause?id=" + id);
    request.send(null);
}

function startTransfer(id) {
    /* Start the transfer with the specified id */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                updateRow(JSON.parse(request.responseText));
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot start download");
            }
        }
    }

    request.open("GET", "transfers/start?id=" + id);
    request.send(null);
}

function removeTransfer(id) {
    /* Remove the transfer with the specified id from the queue */
    
    if (confirm("Do you wish to remove this transfer from the queue?")) {
        var request = new XMLHttpRequest();

        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                if (request.status == 200) {
                    removeRow(id);
                }
                else {
                    var errorString = JSON.parse(request.responseText).error;
                    alert(errorString ? errorString : "Cannot remove download");
                }
            }
        }
    
        request.open("GET", "transfers/remove?id=" + id);
        request.send(null);
    }
}

function pauseAllTransfers() {
    /* Pause all transfers */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                window.location.reload();
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot pause downloads");
            }
        }
    }

    request.open("GET", "transfers/pause");
    request.send(null);
}

function startAllTransfers() {
    /* Start all transfers */
    
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status == 200) {
                window.location.reload();
            }
            else {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot start downloads");
            }
        }
    }

    request.open("GET", "transfers/start");
    request.send(null);
}

function setNextAction() {
    /* Sets the next action to take place after the currently active transfers are finished */
    
    var selector = document.getElementById("actionSelector");
    var action = selector.options[selector.selectedIndex].value;
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status != 200) {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot set property");
            }
        }
    }

    request.open("GET", "transfers/setProperty?property=nextAction&value=" + action);
    request.send(null);
}

function setMaximumConcurrentTransfers() {
    /* Sets the maximum concurrent transfers */
    
    var selector = document.getElementById("concurrentSelector");
    var maximum = selector.options[selector.selectedIndex].value;
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status != 200) {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot set property");
            }
        }
    }

    request.open("GET", "preferences/setProperty?property=maximumConcurrentTransfers&value=" + maximum);
    request.send(null);
}

function setMaximumConnectionsPerTransfer() {
    /* Sets the maximum number of connections per transfer */
    
    var selector = document.getElementById("connectionsSelector");
    var maximum = selector.options[selector.selectedIndex].value;
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status != 200) {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot set property");
            }
        }
    }

    request.open("GET", "preferences/setProperty?property=maximumConnectionsPerTransfer&value=" + maximum);
    request.send(null);
}

function setDownloadRateLimit() {
    /* Sets the download rate limit */
    
    var selector = document.getElementById("rateLimitSelector");
    var limit = selector.options[selector.selectedIndex].value;
    var request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            if (request.status != 200) {
                var errorString = JSON.parse(request.responseText).error;
                alert(errorString ? errorString : "Cannot set property");
            }
        }
    }

    request.open("GET", "preferences/setProperty?property=downloadRateLimit&value=" + limit);
    request.send(null);
}
