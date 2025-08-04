var ws;
function connectWebSocket() {
    ws = new WebSocket("ws://" + window.location.host + "/ws");
    ws.onopen = function () {
        console.log("WebSocket connected!");
    };
    ws.onmessage = function (event) {
        console.log("Message from server:", event.data);
        if (event.data === "reload") {
            console.log("Reloading page...");
            window.location.reload();
        }
    };
    ws.onclose = function (event) {
        console.log("WebSocket disconnected:", event.reason, event.code);
        setTimeout(connectWebSocket, 3000);
    };
    ws.onerror = function (error) {
        console.error("WebSocket error:", error);
        ws.close();
    };
}
connectWebSocket();
