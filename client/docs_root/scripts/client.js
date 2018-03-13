class Client {
    constructor(address, port, callback) {
        try{
            this.state = Client.STATE.CONNECTING;
            this.ws = new WebSocket(address, port);
        } catch(e){
            this.state = Client.STATE.SERVER_OUT;
            console.error(e);
            return;
        };
        this.ws.onopen = ()=>{
            this.state = Client.STATE.IDLE;
            if(callback) callback();
        };
        this.callbacks = {};
        this.ws.onmessage = (data)=>{
            let dv = new DataView(data.msg),
                op = dv.getInt32(0, true);
            switch(op){
            case Client.OP.ECHO:
                console.log("echo: ", new TextDecoder().decode(new Uint8Array(data.msg, 4, dv.byteLength-4)));
                break;
            case Client.OP.LOGIN:
                let result = dv.getUInt8(0, true);
                console.log("login result:", result);
                for(let i in this.callbacks[Client.OP.LOGIN])
                    this.callbacks[Client.OP.LOGIN][i](result);
                break;
            }
            console.log(data);
        };
    }
    echo(msg){
        let encoded_msg = new TextEncoder().encode(msg),
            op_added_msg = new Uint8Array(1 + encoded_msg.length);
        op_added_msg.set([Client.OP.ECHO]);
        op_added_msg.set(encoded_msg, 1);
        this.ws.send(op_added_msg);
    }
};
Client.OP = Object.freeze({
    ECHO: 0,
    JOIN: 1,
    LOGIN: 2,
});
Client.STATE = Object.freeze({
    CONNECTING: Symbol("CONNECTING"),
    SERVER_OUT: Symbol("CONNECT_FAIL"),
    IDLE: Symbol("IDLE"),
    TRY_LOGIN: Symbol("TRY_LOGIN"),
    RUNNING: Symbol("RUNNING"),
});
