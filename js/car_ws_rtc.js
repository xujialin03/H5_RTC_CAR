const { WebSocketServer } = require('ws');
const http = require('http');
const fs = require('fs');
const express = require('express')
const app = express()
const ws_service = new WebSocketServer({ port: 8080 });
let conID='conClient';
let phoneID='phoneClient';
let carID='carClient'
var client_list = [];
app.get('/', function (req, res) {
    res.send('Hello World!')
})
app.get('/con', function (req, res) {
    res.sendFile(__dirname + '/con_vue.html')
})
app.get('/phone', (req, res) => {
    res.sendFile(__dirname + '/phone_ws.html')
})
ws_service.on('connection', function connection(ws, req) {
    // console.log('conn')
    const ip = req.headers['x-forwarded-for'].split(',')[0].trim();
    console.log(ip)
    ws.on('message', function message(data) {
        // console.log('received: %s', data);
        handlerMessage(ws, data)
    });
    ws.on('pong', () => {
        console.log('pong event')
    })
    ws.on('close',()=>{
        console.log('用户断开')
    })
});
ws_service.on('close', () => {
    console.log('ws_service close')
})
function handlerMessage(ws, msg) {
    let msg_obj = ""
    try {
        msg_obj = JSON.parse(msg);
    }
    catch (error) {
        console.log('msg 解析错误:', msg)
    }
    if (msg_obj.com === "ping") {
        // console.log('ping event')
        // console.log("XXX",msg_obj.data.id)
        
        let client = getClientByID(msg_obj.data.id)
        if (client==null)
        {
            // console.log('ping null',msg_obj.data.id)
        }
        else
        {
            client.isAlive = true;
        }
        // console.log(client)
        
    }
    if (msg_obj.com === "phone") {
        phoneID=createid();
        let phone_client = { id: phoneID, conn: ws, isAlive: true }
        client_list.push(phone_client)
        let retu_msg = {
            com: 'conn_success',
            data: { id:phoneID }
        }
        phone_client.conn.send(JSON.stringify(retu_msg))
        if (getClientByID(conID) != null) {
            let msg2 = { com: 'phone_ready', data: { id: phoneID} }
            getClientByID(conID).conn.send(JSON.stringify(msg2))
        }
    }
    if (msg_obj.com === "con") {
        conID=createid();
        let con_client = { id:conID, conn: ws, isAlive: true }
        client_list.push(con_client)
        let retu_msg = {
            com: 'conn_success',
            data: { id: conID }
        }
        con_client.conn.send(JSON.stringify(retu_msg))
        if (getClientByID(phoneID) != null) {
            let msg2 = { com: 'phone_ready', data: { id: phoneID} }
            con_client.conn.send(JSON.stringify(msg2))
            // console.log('msg2')
        }
        if (getClientByID(carID) != null) {
            let msg2 = { com: 'car_ready', data: { id: carID } }
            con_client.conn.send(JSON.stringify(msg2))
        }
    }
    if (msg_obj.com == "car") {
        console.log('car coming;')
        carID=createid();
        let car_client = { id: carID, conn: ws, isAlive: true }
        client_list.push(car_client)
        console.log(client_list.length)
        let retu_msg = {
            com: 'conn_success',
            data: { id: carID }
        }
        car_client.conn.send(JSON.stringify(retu_msg))
        if (getClientByID(conID) != null) //通知控制端car加进来
        {
            let msg2 = { com: 'car_ready', data: { id: carID } }
            getClientByID(conID).conn.send(JSON.stringify(msg2))
        }
        
    }
    if (msg_obj.com === "request_offer") {
        getClientByID(phoneID).conn.send(JSON.stringify(msg_obj))
    }
    if (msg_obj.com === "to") {
        // console.log('to event', msg_obj.to)
        let target = getClientByID(msg_obj.to)
        if (target)
        {
            target.conn.send(JSON.stringify(msg_obj))
        }
        else
        {
            console.error("to msg target null:",msg_obj)
        }
        
    }
    if (msg_obj.com === "sdp") {
        console.log('spd event', msg_obj.to)
        let target = getClientByID(msg_obj.to)
        if (target)
        {
            target.conn.send(JSON.stringify(msg_obj))
        }
        else
        {
            console.log('sdp error:',msg_obj)
        }
        
    }
    if (msg_obj.com === "ice candidates") {
        console.log('ice candidates event ->', msg_obj.to)
        let target = getClientByID(msg_obj.to)
        if (target==null)
        {
            console.log('ice candidates error',msg_obj)
        }
        else
        {
            target.conn.send(JSON.stringify(msg_obj))
        }
        
    }
}
function getClientByID(tid) {

    // console.log(client_list.length)
    for (var i=0;i<client_list.length;i++)
    {
        // console.log("-->",client_list[i]['id'],tid,client_list[i]['id']==tid)
        if (client_list[i].id==tid)
        {
            return client_list[i];
        }
    }

    return null;
}
//心跳处理
const aliveHandler = function () {
    let msg2 = { com: 'ping' }
    let leave_client=client_list.filter(client => client.isAlive == false);//已经离开的客户端
    client_list = client_list.filter(client => client.isAlive == true);//还离开的客户端
    // console.log(client_list.length)
    client_list.forEach(element => {
        for (var i=0;i<leave_client.length;i++)
        {
            let client_event_msg_data={
                com:'client_leave',
                data:{id:leave_client[i].id}
            }
            console.log('client_leave_msg',leave_client[i].id)
            element.conn.send(JSON.stringify(client_event_msg_data))
        }
        element.isAlive = false;
    });
    leave_client.forEach(element=>{
        element=null;
    })

}
const aliveInterval = setInterval(aliveHandler, 300);
//生成随机数id方法
const createid=function()
{
    return Math.floor((Math.random()*100000));
}


const http_port = 3000
app.listen(http_port, () => {
    console.log(`express server listen at http://localhost:${http_port}`)
})