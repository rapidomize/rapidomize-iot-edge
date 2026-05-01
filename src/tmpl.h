#ifndef RPZ_TMPL_H_
#define RPZ_TMPL_H_

#include <Arduino.h>

namespace rpz{

const char *main_tmpl = R"(
<!DOCTYPE html>
<html lang='en'>
    <head>
        <meta name='viewport' content='width=device-width'>
        <title>Rapidomize IoT Edge</title>
        <style>
            h3{
                margin-top: 10px;
            }
            .fx{
                display: flex;
            }
            .fx-g{
                flex: 1 1 auto;
            }
            .row{
                display: flex;
                flex-direction: row;
            }
            .column{
                display: flex;
                flex-direction: column;
            }
            .container{
                max-width: 800px;
                margin: auto;
                padding: 10px 20px;
            }
            .pos-r{
                position: relative;
            }
            .pos-a{
                position: absolute;
            }
            .card{
                border: 1px solid #d1d1d1; 
                border-radius: 6px;
                padding: 10px;
                min-width: fit-content;
            }
            .cards{
                display: grid;
                grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
                grid-gap: 10px;
            }
            .g-10{
                grid-gap: 10px;
            }
            .brdr{
                border-radius: 6px;
                padding: 10px;
            }
            .mt-10{
                margin-top: 10px;
            }
            .mt-30{
                margin-top: 30px;
            }
            .mb-40{
                margin-bottom: 40px;
            }
            .tabs {
                display: flex;
                flex-wrap: wrap;
                font-family: sans-serif;
                margin-bottom: 300px;
            }
            .tabs__label {
                padding: 10px 16px;
                cursor: pointer;
            }
            .tabs__radio {
                display: none;
            }
            .tabs__content {
                order: 1;
                width: 100%%;
                height: 100%%;
                border-top: 2px solid #dddddd;
                display: none;
            }
            .tabs__radio:checked + .tabs__label {
                font-weight: bold;
                color: #37a000;
                border-bottom: 3px solid #37a000;
            }
            .tabs__radio:checked + .tabs__label + .tabs__content {
                display: initial;
            }
            input, select{
                padding: 4px 12px;
                font-size: 14px;
                border: 1px solid #d1d1d1;
                border-radius: 6px;
                transition: border-color 0.15s ease-in-out, box-shadow 0.15s ease-in-out;
                min-width: fit-content;
                width: auto;
            }
            .switch { position: relative; display: inline-block; width: 40px; height: 14px; }
            .switch input { opacity: 0; width: 0; height: 0; }
            .slider {
                position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;
                background-color: #ccc; transition: .2s; border-radius: 34px;
            }
            .slider:before {
                position: absolute; content: ""; height: 22px; width: 22px;
                left: 0px; bottom: -5px; 
                transition: .2s; border-radius: 50%%;
                background-color: #eee;
                border: 1px solid #eee;
                box-shadow: 0 2px 4px rgba(0,0,0,0.2);
            }
            .switch input:checked + .slider { background-color: #37a000; }
            .switch input:checked + .slider:before { background-color: #fff; border-color: #37a000; transform: translateX(22px); }
            input[type=checkbox]{
                display: block;
                width: 1em;
                height: 1em;
                margin-top: 0.25em;
                background-color: #fff;
                background-repeat: no-repeat;
                background-position: center;
                background-size: contain;
                -webkit-appearance: none;
                -moz-appearance: none;
                appearance: none;
                -webkit-print-color-adjust: exact;
            }
            input[type=checkbox]:checked {
                background-color:  #37a000;
                border-color: #37a000;
                background-image: url("data:image/svg+xml,%%3csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20'%%3e%%3cpath fill='none' stroke='%%23fff' stroke-linecap='round' stroke-linejoin='round' stroke-width='3' d='M6 10l3 3l6-6'/%%3e%%3c/svg%%3e");
            }
            .sv-btn{
                 width: 26px;
                height: 26px;
                border: 0;
                right: 0px;
                padding: 1px;
                color: #fff;
                background-color: #37a000;
                cursor: pointer;
                display: inline-flex;
                font-size: 24px;
            }
            .sv-btn:hover { background-color: #297701; }
            .sv-btn:active{
                background-color: #37a000;
                border-color: #37a000;
            }
            .sv-btn:disabled {
                background-color: rgb(156, 156, 156);
                border-color: rgb(156, 156, 156);  
            }
            .ptitle{
                border: 0; 
                font-size: 20px; 
                background-color: #fff;
            }
            .footer {
                position: fixed;
                left: 0;
                bottom: 0;
                width: 100%%;
                background: rgb(234 234 234);
                text-align: center;
                box-shadow: 0 1px 6px rgba(57,73,76,0.35);
                -webkit-box-shadow: 0 1px 6px rgba(57,73,76,0.35);
            }
        </style>
    </head>
    <body class="container column">
        <div class="row" style="align-items: end; margin-bottom: 20px;">
            <img class="img-fluid" src="https://rapidomize.io/img/logo-text.svg" alt="Rapidomize | Low-Code Service Creation Platform" loading="lazy" 
                style="height:auto;max-width: 100%%; max-height: 56px;margin-right: 30px;">
            <h1>IoT Edge</h1>
        </div>
        <div class="row">
            <div id="msg" class="fx-g">%s</div>
            <form id="logout" action="/logout" method="post">
                <input type="submit" value='Logout' style="width: 100px; height: 25px;display:%s">
            </form>
        </div>
        <progress id="progBar" value="0" max="100" style="display: none; background-color: #37a000;"></progress>
        <div class="tabs">
            %s
            %s
            %s
            %s
            %s
        </div>
        <div class="footer">
            <p>Copyright &copy; Rapidomize LLC. All Rights Reserved.</p>
        </div>   
        <script>
            const forms = document.getElementsByTagName('form');
            const msg = document.getElementById('msg');
            const progressBar = document.getElementById("progBar");
            for(let i=0; i < forms.length; i++){
                forms[i].addEventListener('submit', function (event) {
                    event.preventDefault();
                    event.stopPropagation();

                    msg.textContent = '';

                    const xhr = new XMLHttpRequest();

                    xhr.addEventListener("load", function(evt) {
                        const contentType = xhr.getResponseHeader("Content-Type");
                        if (!contentType || !contentType.includes("application/json")) {
                            msg.textContent = `Unexpected error! status: ${xhr.status} - ${xhr.statusText}`;
                            msg.setAttribute('style', 'color: red;');
                            xhr.abort();
                        }
                        const data = xhr.response;
                        //const data = JSON.parse(xhr.response);
                        switch(xhr.status){
                            case 200: 
                                if(data['urlp']){
                                    window.location.href = data['urlp'];
                                }else if(data['url']){
                                    window.location = data['url'];
                                }
                                console.log('Success:', data);
                                msg.textContent = data['err']?data['err']:'Success';
                                msg.setAttribute('style', 'color: green;');
                                break;
                            default:
                                console.log('Failed:', data);
                                msg.textContent = data['err']?data['err']:'Error';
                                msg.setAttribute('style', 'color: red;');
                        }
                        progressBar.style.display = 'none';
                        const btn = forms[i].querySelector('.sv-btn');
                        btn.disabled = true;
                    }, false);

                    xhr.addEventListener("error", function(evt) {
                        console.error('Network error occurred');
                        console.error('Ready state:', xhr.readyState);
                        console.error('Status:', xhr.status); // Will likely be 0
                        console.error('Status text:', xhr.statusText);
                        msg.textContent = error;
                        msg.setAttribute('style', 'color: red;');
                        progressBar.style.display = 'none';
                        const btn = forms[i].querySelector('.sv-btn');
                        btn.disabled = true;
                    }, false);

                    xhr.open("POST", forms[i].action);
                    xhr.responseType = 'json';
                    
                    let body = new FormData(event.target);
                    if(forms[i].action.indexOf("/fwfile") < 0){
                        // Convert the JavaScript object to a JSON string
                        const frmData = Object.fromEntries(body.entries());
                        body = JSON.stringify(frmData)
                        console.log('data', frmData);
                        xhr.setRequestHeader('Content-Type', 'application/json');
                    }else{
                        progressBar.style.display = 'block';
                        xhr.upload.addEventListener("progress", function(evt) {
                            if (evt.lengthComputable) {
                                progressBar.value = (evt.loaded / evt.total) * 100; 
                            }
                        }, false);
                    }

                    xhr.send(body);
                });

                if(forms[i].action.indexOf('/peri') >= 0){
                    let btn = forms[i].querySelector('.sv-btn');
                    btn.disabled = true;
                    forms[i].addEventListener('change', function() {
                        btn.disabled = false;
                        msg.textContent = 'You must save your changes';
                        msg.setAttribute('style', 'color: green;');
                    });                    
                }
            }
            const evtSource = new EventSource("/evts");
            evtSource.onmessage = (event) => {
                const eventList = document.getElementById("evts");
                if(eventList.children.length > 100)
                    eventList.firstChild.remove();
                const newElement = document.createElement("li");
                newElement.textContent = event.data;
                eventList.appendChild(newElement);
            };
            evtSource.onerror = (err) => {
                console.error("EventSource failed:", err);
            };
        </script>
    </body>
</html>
)";

const char *tabs_tmpl = R"(
<input type="radio" class="tabs__radio" name="atabs" id="tab5">
<label for="tab5" class="tabs__label">Firmware</label>
<div class="tabs__content">
    <h2>Firmware Upgrade</h2>
    <p>Upgrade firmware using a local file or remote url.</p>
    <!--<form action="/fwurl" method="post" class="card column brd">
        <h4>Use A Remote URL (OTA)</h4>
        <input type="text" name="fw_url" value="https://github.com/rapidomize/rapidomize-iot-edge/releases/latest/download/rapidomize-iot-edge-%s.bin" class="fx-g" required>
        <input type="submit"  value="Update" class="brdr" style="margin: 20px auto; width: 200px;">
    </form>-->
    <form action="/fwfile" method="post" enctype="multipart/form-data" class="card column brd mt-30">
        <h4>Use A Local File</h4>
        <input type="file" name="fw_file" required>
        <input type="submit"  value="Update" class="brdr" style="margin: 20px auto; width: 200px;">
    </form>
</div>
<input type="radio" class="tabs__radio" name="atabs" id="tab6">
<label for="tab6" class="tabs__label">Logs</label>
<div class="tabs__content">
    <h2>Logs</h2>
    <p>Last 100 events & messages</p>
    <div id="evts" class="card column" style="padding-left: 20px;height: 60vh;overflow-y: auto;"></div>
</div>
<input type="radio" class="tabs__radio" name="atabs" id="tab7">
<label for="tab7" class="tabs__label">Other</label>
<div class="tabs__content">
    <h2>Other Settings</h2>
    <form id="prefs" action="/prefs" method="post" class="column card">
        <div class="column">
            <!--<h4>Credentials</h4>-->
            <table>
                <tr><td style="width: 200px;">New Password</td><td class="fx"><input type="password" name="pwd" minlength="8" style="flex-grow: 1"></td></tr>
                <tr><td>Confirm Password</td><td class="fx"><input type="password" name="cpwd" minlength="8" style="flex-grow: 1"></td></tr>
            </table>
        </div>
        <table class="mt-30">
            <tr><td style="width: 200px;">Access Point (iot_edge)</td><td><input type="checkbox" name="ap" %s></td></tr>
            <tr><td style="width: 200px;">WiFi</td><td><input type="checkbox" name="wifi" %s %s></td></tr>
        </table>
        <input type="submit" value='Save Settings' class="brdr" style="margin: 20px auto; width: 200px;">
    </form>
    <form id="reset" action="/reset" method="post" class="column card mt-30" style="color: red;">
        <p>Reset the IoT Edge to it's factory settings</p>
        <input type="submit" value='Factory Reset' class="brdr" style="margin: 20px auto; width: 200px;color: red;">
    </form>
</div>
)";


const char *auth_tmpl = R"(
<input type="radio" class="tabs__radio" name="atabs" id="tab0" checked>
<label for="tab0" class="tabs__label">Auth</label>
<div class="tabs__content">
    <form  action="/auth" method="post" class="column"  class="column">
        <div class="column card mt-30" style="grid-gap: 5px;">
            <table style="margin: auto;width: 350px;">
                <tr><td>Username</td><td class="fx ml-20"><input type="text" name="usr"  style="flex-grow: 1" required></td></tr>
                <tr><td>Password</td><td class="fx ml-20"><input type="password" name="pwd"  style="flex-grow: 1" required></td></tr>
            </table>
        </div>
        <input type="submit" value='Login' class="brdr" style="margin: 20px auto; width: 200px;">
    </form>
</div>
)";

const char *dash_tmpl = R"(
<input type="radio" class="tabs__radio" name="atabs" id="tab1" checked>
<label for="tab1" class="tabs__label">Home</label>
<div class="tabs__content">
    <table class="card mt-10">
        <tr><td>Model</td><td>%s</td></tr>
        <tr><td>Firmware Version</td><td>%s</td></tr>
        <tr><td style="width: 150px;">CPU Freq</td><td>%dMHz</td></tr>
        <tr><td>Image Size</td><td>%dKB</td></tr>
        <tr><td>WiFi IP</td><td>%s</td></tr>
        <tr><td>Ethernet IP</td><td>%s</td></tr>
    </table>
    <div class="cards mt-10">
        %s
    </div>
</div>
)";

const char *netwrk_tmpl = R"(
<input type="radio" class="tabs__radio" name="atabs" id="tab2">
<label for="tab2" class="tabs__label">Network</label>
<div class="tabs__content">
    <h2>Network Settings</h2>
    <p>Network connectivity uses DHCP</p>
    <form action="/wifi" method="post" class="column card mt-30">
        <h2>WiFi</h2>
        <div class="row mt-10"><div style="margin-right: 10px;">IP:</div><div>%s</div></div>
        <div class="row mt-10"><div style="margin-right: 10px;">MAC:</div><div>%s</div></div>
        <p>Select a WiFi network and provide it's credentials</p>
        <label style="padding: 10px;">SSIDs:
            <div class="column">
                %s
            </div>
        </label>
        <label style="margin-top: 10px;">WiFi Password:
            <input type="password" name="pwd" required>
        </label>
        <input type="submit"  value="Connect" class="brdr" style="margin: 20px auto; width: 200px;">
    </form>
    <div class="column card mt-30">
        <h2>Ethernet</h2>
        <div class="row mt-10"><div style="margin-right: 10px;">IP:</div><div>%s</div></div>
        <div class="row mt-10"><div style="margin-right: 10px;">MAC:</div><div>%s</div></div>
    </div>
</div>
)";

const char *mqtt_tmpl = R"(
<input type="radio" class="tabs__radio" name="atabs" id="tab3">
<label for="tab3" class="tabs__label">MQTT</label>
<div class="tabs__content">
    <h2>MQTT Broker Settings</h2>
    <p>Specify MQTT Broker details.</p>
    <form action="/mqtt" method="post" class="column card">
        <div class="column" style="grid-gap: 5px;">
            <table>
                <tr><td>Host</td><td class="fx"><input type="text" name="host" value="%s" class="fx-g" required></td></tr>
                <tr><td>Port</td><td><input type="number" name="port" value="%d" required></td></tr>
                <tr><td>TLS/SSL</td><td><input type="checkbox" name="tls" %s></td></tr>
                <tr><td>Client ID</td><td class="fx"><input type="text" name="clientId" value="%s"  class="fx-g" required></td></tr>
                <tr><td>Username</td><td class="fx"><input type="text" name="username" value="%s"  class="fx-g"></td></tr>
                <tr><td>Password</td><td class="fx"><input type="password" name="password" value="%s" class="fx-g"></td></tr>
                <tr><td style="width: 150px;">Publishing Topic</td><td class="fx"><input type="text" name="topic"  value="%s" class="fx-g" required></td></tr>
                <tr><td>Version</td><td>
                    <select name="ver" value="%s">
                        <option value="3.1.1">3.1.1</option>
                        <option value="5.0">5.0</option>
                    </select>
                </td></tr>
                <tr><td>QoS</td><td>
                    <select name="qos" value="%d">
                        <option value="0">0</option>
                        <option value="1">1</option>
                        <option value="2">2</option>
                    </select>  
                </td></tr>
                <tr><td>Rapidomize Format</td><td><input type="checkbox" name="rpzfmt" %s></td></tr>
            </table>
        </div>
        <input type="submit" value="Connect" class="brdr" style="margin: 20px auto; width: 200px;">
    </form>
</div>
)";

const char *peri_tmpl = R"(
<input type="radio" class="tabs__radio" name="atabs" id="tab4">
<label for="tab4" class="tabs__label">Peripherals</label>
<div class="tabs__content">
    <h2>Peripherals</h2>
    <p>Peripheral hardware configuration</p>
    <div class="cards g-10">
        %s
    </div>
</div>
)";

const char *ssid_tmpl = R"(<div><input type="radio" name="ssid" value="%s" %s> <label>%s</label></div>)";

} // namespace rpz




#endif //RPZ_TMPL_H_
