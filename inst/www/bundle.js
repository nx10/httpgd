(()=>{"use strict";var t={474:(t,e,n)=>{t.exports=n.p+"103f47f4ae284eb62b37.svg"}},e={};function n(i){var o=e[i];if(void 0!==o)return o.exports;var s=e[i]={exports:{}};return t[i](s,s.exports,n),s.exports}n.g=function(){if("object"==typeof globalThis)return globalThis;try{return this||new Function("return this")()}catch(t){if("object"==typeof window)return window}}(),(()=>{var t;n.g.importScripts&&(t=n.g.location+"");var e=n.g.document;if(!t&&e&&(e.currentScript&&(t=e.currentScript.src),!t)){var i=e.getElementsByTagName("script");i.length&&(t=i[i.length-1].src)}if(!t)throw new Error("Automatic publicPath is not supported in this browser");t=t.replace(/#.*$/,"").replace(/\?.*$/,"").replace(/\/[^\/]+$/,"/"),n.p=t})(),(()=>{var t=function(t,e,n,i){return new(n||(n=Promise))((function(o,s){function a(t){try{c(i.next(t))}catch(t){s(t)}}function r(t){try{c(i.throw(t))}catch(t){s(t)}}function c(t){var e;t.done?o(t.value):(e=t.value,e instanceof n?e:new n((function(t){t(e)}))).then(a,r)}c((i=i.apply(t,e||[])).next())}))},e=function(t,e){var n,i,o,s,a={label:0,sent:function(){if(1&o[0])throw o[1];return o[1]},trys:[],ops:[]};return s={next:r(0),throw:r(1),return:r(2)},"function"==typeof Symbol&&(s[Symbol.iterator]=function(){return this}),s;function r(s){return function(r){return function(s){if(n)throw new TypeError("Generator is already executing.");for(;a;)try{if(n=1,i&&(o=2&s[0]?i.return:s[0]?i.throw||((o=i.return)&&o.call(i),0):i.next)&&!(o=o.call(i,s[1])).done)return o;switch(i=0,o&&(s=[2&s[0],o.value]),s[0]){case 0:case 1:o=s;break;case 4:return a.label++,{value:s[1],done:!1};case 5:a.label++,i=s[1],s=[0];continue;case 7:s=a.ops.pop(),a.trys.pop();continue;default:if(!((o=(o=a.trys).length>0&&o[o.length-1])||6!==s[0]&&2!==s[0])){a=0;continue}if(3===s[0]&&(!o||s[1]>o[0]&&s[1]<o[3])){a.label=s[1];break}if(6===s[0]&&a.label<o[1]){a.label=o[1],o=s;break}if(o&&a.label<o[2]){a.label=o[2],a.ops.push(s);break}o[2]&&a.ops.pop(),a.trys.pop();continue}s=e.call(t,a)}catch(t){s=[6,t],i=0}finally{n=o=0}if(5&s[0])throw s[1];return{value:s[0]?s[1]:void 0,done:!0}}([s,r])}}},i=n(474),o=function(){function n(t,e){this.httpHeaders=new Headers,this.http="http://"+t,this.ws="ws://"+t,this.httpSVG=this.http+"/svg",this.httpState=this.http+"/state",this.httpClear=this.http+"/clear",this.httpRemove=this.http+"/remove",this.httpPlots=this.http+"/plots",e?(this.useToken=!0,this.token=e,this.httpHeaders.set("X-HTTPGD-TOKEN",this.token)):(this.useToken=!1,this.token="")}return n.prototype.svg_index=function(t,e,n,i){var o=this.svg_ext(e,n,i);return o.searchParams.append("index",t.toString()),o},n.prototype.svg_id=function(t,e,n,i){var o=this.svg_ext(e,n,i);return o.searchParams.append("id",t),o},n.prototype.svg_ext=function(t,e,n){var i=new URL(this.httpSVG);return t&&i.searchParams.append("width",Math.round(t).toString()),e&&i.searchParams.append("height",Math.round(e).toString()),this.useToken&&i.searchParams.append("token",this.token),n&&i.searchParams.append("c",n),i},n.prototype.remove_index=function(t){var e=new URL(this.httpRemove);return e.searchParams.append("index",t.toString()),e},n.prototype.get_remove_index=function(n){return t(this,void 0,void 0,(function(){return e(this,(function(t){switch(t.label){case 0:return[4,fetch(this.remove_index(n).href,{headers:this.httpHeaders})];case 1:return[2,t.sent()]}}))}))},n.prototype.remove_id=function(t){var e=new URL(this.httpRemove);return e.searchParams.append("id",t),e},n.prototype.get_remove_id=function(n){return t(this,void 0,void 0,(function(){return e(this,(function(t){switch(t.label){case 0:return[4,fetch(this.remove_id(n).href,{headers:this.httpHeaders})];case 1:return[2,t.sent()]}}))}))},n.prototype.get_plots=function(){return t(this,void 0,void 0,(function(){return e(this,(function(t){switch(t.label){case 0:return[4,fetch(this.httpPlots,{headers:this.httpHeaders})];case 1:return[4,t.sent().json()];case 2:return[2,t.sent()]}}))}))},n.prototype.get_clear=function(){return t(this,void 0,void 0,(function(){return e(this,(function(t){switch(t.label){case 0:return[4,fetch(this.httpClear,{headers:this.httpHeaders})];case 1:return[2,t.sent()]}}))}))},n.prototype.get_state=function(){return t(this,void 0,void 0,(function(){return e(this,(function(t){switch(t.label){case 0:return[4,fetch(this.httpState,{headers:this.httpHeaders})];case 1:return[4,t.sent().json()];case 2:return[2,t.sent()]}}))}))},n.prototype.new_websocket=function(){return new WebSocket(this.ws)},n}(),s=function(){function t(t,e,n){this.mode=0,this.pausePoll=!1,this.disconnected=!0,this.api=new o(t,e),this.allowWebsockets=n||!1}return t.prototype.open=function(){0==this.mode&&this.start(3)},t.prototype.close=function(){0!=this.mode&&this.start(0)},t.prototype.start=function(e){var n=this;if(this.mode!=e)switch(e){case 1:console.log("Start POLL"),this.clearWebsocket(),this.clearPoll(),this.pollHandle=setInterval((function(){return n.poll()}),t.INTERVAL_POLL),this.mode=e;break;case 2:console.log("Start SLOWPOLL"),this.clearWebsocket(),this.clearPoll(),this.pollHandle=setInterval((function(){return n.poll()}),t.INTERVAL_POLL_SLOW),this.mode=e;break;case 3:if(!this.allowWebsockets){this.start(1);break}console.log("Start WEBSOCKET"),this.clearPoll(),this.clearWebsocket(),this.socket=this.api.new_websocket(),this.socket.onmessage=function(t){return n.onWsMessage(t.data)},this.socket.onopen=function(){return n.onWsOpen()},this.socket.onclose=function(){return n.onWsClose()},this.socket.onerror=function(){return console.log("Websocket error")},this.mode=e,this.poll();break;case 0:this.clearWebsocket(),this.clearPoll(),this.mode=e}},t.prototype.clearPoll=function(){this.pollHandle&&clearInterval(this.pollHandle)},t.prototype.clearWebsocket=function(){this.socket&&(this.socket.onclose=function(){},this.socket.close())},t.prototype.poll=function(){var t=this;this.pausePoll||this.api.get_state().then((function(e){t.setDisconnected(!1),2===t.mode&&t.start(3),t.pausePoll||t.checkState(e)})).catch((function(e){console.warn(e),t.setDisconnected(!0)}))},t.prototype.onWsMessage=function(t){if(t.startsWith("{")){var e=JSON.parse(t);this.checkState(e)}else console.log("Unknown WS message: "+t)},t.prototype.onWsClose=function(){console.log("Websocket closed"),this.setDisconnected(!0)},t.prototype.onWsOpen=function(){console.log("Websocket opened"),this.setDisconnected(!1)},t.prototype.setDisconnected=function(t){var e;this.disconnected!=t&&(this.disconnected=t,this.disconnected?this.start(2):this.start(3),null===(e=this.connectionChanged)||void 0===e||e.call(this,t))},t.prototype.checkState=function(t){var e;this.lastState&&this.lastState.active===t.active&&this.lastState.hsize===t.hsize&&this.lastState.upid===t.upid||(this.lastState=t,null===(e=this.remoteStateChanged)||void 0===e||e.call(this,t))},t.INTERVAL_POLL=500,t.INTERVAL_POLL_SLOW=5e3,t}(),a=function(){function t(){this.index=-1,this.width=0,this.height=0,this.last_id="",this.last_width=0,this.last_height=0}return t.prototype.navigate=function(t){this.data&&(this.index=(this.data.plots.length+this.index+t)%this.data.plots.length)},t.prototype.jump=function(t){this.data&&(this.index=(this.data.plots.length+t)%this.data.plots.length)},t.prototype.jump_id=function(t){if(this.data)for(var e=0;e<this.data.plots.length;e++)if(t===this.data.plots[e].id){this.index=e;break}},t.prototype.resize=function(t,e){this.width=t,this.height=e},t.prototype.next=function(t,e){return this.data&&0!=this.data.plots.length?this.last_id!==this.data.plots[this.index].id||Math.abs(this.last_width-this.width)>.1||Math.abs(this.last_height-this.height)>.1?t.svg_id(this.data.plots[this.index].id,this.width,this.height,e).href:void 0:i},t.prototype.update=function(t){this.data=t,this.index=t.plots.length-1},t.prototype.id=function(){if(this.data&&0!=this.data.plots.length)return this.data.plots[this.index].id},t.prototype.indexStr=function(){return this.data?Math.max(0,this.index+1)+"/"+this.data.plots.length:"0/0"},t}(),r=function(){function t(e,n,i){var o=this;this.navi=new a,this.plotUpid=-1,this.scale=t.SCALE_DEFAULT,this.deviceActive=!0,this.image=void 0,this.sidebar=void 0,this.resizeBlocked=!1,this.connection=new s(e,n,i),this.connection.remoteStateChanged=function(t){return o.serverChanges(t)},this.connection.connectionChanged=function(t){var e;return null===(e=o.onDisconnectedChange)||void 0===e?void 0:e.call(o,t)}}return t.prototype.init=function(t,e){var n,i,o=this;this.image=t,this.sidebar=e,this.connection.open(),this.checkResize(),document.addEventListener("visibilitychange",(function(){document.hidden||o.updateImage("v")}),!1),null===(n=this.onIndexStringChange)||void 0===n||n.call(this,this.navi.indexStr()),null===(i=this.onZoomStringChange)||void 0===i||i.call(this,this.getZoomString()),console.log("initial update plots"),this.updatePlots(!0)},t.prototype.updatePlots=function(t){var e=this;void 0===t&&(t=!1),this.connection.api.get_plots().then((function(n){var i;e.navi.update(n),null===(i=e.onIndexStringChange)||void 0===i||i.call(e,e.navi.indexStr()),e.updateSidebar(n,t),e.updateImage()}))},t.prototype.updateImage=function(t){if(this.image){var e=this.navi.next(this.connection.api,this.plotUpid+(t||""));e&&(console.log("update image"),this.image.src=e)}},t.prototype.updateSidebar=function(t,e){var n=this;if(void 0===e&&(e=!1),this.sidebar){for(var i=0;i<this.sidebar.children.length;)i>=t.plots.length||this.sidebar.children[i].getAttribute("data-pid")!==t.plots[i].id?this.sidebar.removeChild(this.sidebar.children[i]):i++;for(var o=function(){var e=t.plots[i],o=document.createElement("div");o.setAttribute("data-pid",e.id);var a=document.createElement("a");a.innerHTML="&#10006;",a.onclick=function(){n.connection.api.get_remove_id(e.id),n.updatePlots()};var r=document.createElement("img");o.classList.add("history-item"),r.setAttribute("src",s.connection.api.svg_id(e.id).href),o.onclick=function(){var t;n.navi.jump_id(e.id),null===(t=n.onIndexStringChange)||void 0===t||t.call(n,n.navi.indexStr()),n.updateImage()},o.appendChild(r),o.appendChild(a),s.sidebar.appendChild(o)},s=this;i<t.plots.length;++i)o();e&&(this.sidebar.scrollTop=this.sidebar.scrollHeight)}},t.prototype.serverChanges=function(t){this.setDeviceActive(!t.active);var e=this.plotUpid;this.plotUpid=t.upid,e!==t.upid&&this.updatePlots(!0)},t.prototype.setDeviceActive=function(t){var e;this.deviceActive!==t&&(this.deviceActive=t,null===(e=this.onDeviceActiveChange)||void 0===e||e.call(this,t))},t.prototype.zoomIn=function(){var e;this.scale-t.SCALE_STEP>.05&&(this.scale-=t.SCALE_STEP),null===(e=this.onZoomStringChange)||void 0===e||e.call(this,this.getZoomString()),this.checkResize()},t.prototype.zoomOut=function(){var e;this.scale+=t.SCALE_STEP,null===(e=this.onZoomStringChange)||void 0===e||e.call(this,this.getZoomString()),this.checkResize()},t.prototype.zoomReset=function(){var e;this.scale=t.SCALE_DEFAULT,null===(e=this.onZoomStringChange)||void 0===e||e.call(this,this.getZoomString()),this.checkResize()},t.prototype.getZoomString=function(){return Math.ceil(t.SCALE_DEFAULT/this.scale*100)+"%"},t.prototype.navPrevious=function(){var t;this.navi.navigate(-1),null===(t=this.onIndexStringChange)||void 0===t||t.call(this,this.navi.indexStr()),this.updateImage()},t.prototype.navNext=function(){var t;this.navi.navigate(1),null===(t=this.onIndexStringChange)||void 0===t||t.call(this,this.navi.indexStr()),this.updateImage()},t.prototype.navNewest=function(){var t;this.navi.jump(-1),null===(t=this.onIndexStringChange)||void 0===t||t.call(this,this.navi.indexStr()),this.updateImage()},t.prototype.navClear=function(){this.connection.api.get_clear(),this.updatePlots()},t.prototype.navRemove=function(){var t=this.navi.id();t&&(this.connection.api.get_remove_id(t),this.updatePlots())},t.downloadURL=function(t,e){var n=document.createElement("a");n.href=t,e&&(n.download=e),document.body.appendChild(n),n.click(),document.body.removeChild(n)},t.prototype.downloadPlotSVG=function(e){var n=this;this.navi.id()&&fetch(e.src).then((function(t){return t.blob()})).then((function(e){t.downloadURL(URL.createObjectURL(e),"plot_"+n.navi.id()+".svg")}))},t.imageTempCanvas=function(t,e){var n=document.createElement("canvas");document.body.appendChild(n);var i=t.getBoundingClientRect();n.width=i.width,n.height=i.height;var o=n.getContext("2d");o&&(o.drawImage(t,0,0,n.width,n.height),e(n),document.body.removeChild(n))},t.prototype.downloadPlotPNG=function(e){var n=this;e&&this.navi.id()&&t.imageTempCanvas(e,(function(e){var i=e.toDataURL("image/png").replace("image/png","image/octet-stream");t.downloadURL(i,"plot_"+n.navi.id()+".png")}))},t.prototype.copyPlotPNG=function(e){e&&this.navi.id()&&navigator.clipboard&&t.imageTempCanvas(e,(function(t){t.toBlob((function(t){var e,n;t&&(null===(n=(e=navigator.clipboard).write)||void 0===n||n.call(e,[new ClipboardItem({"image/png":t})]))}))}))},t.prototype.checkResize=function(){if(this.image){var t=this.image.getBoundingClientRect();this.navi.resize(t.width*this.scale,t.height*this.scale),this.updateImage()}},t.prototype.resize=function(){var e=this;this.resizeBlocked||(this.resizeBlocked=!0,setTimeout((function(){e.checkResize(),e.resizeBlocked=!1}),t.COOLDOWN_RESIZE))},t.COOLDOWN_RESIZE=200,t.SCALE_DEFAULT=.8,t.SCALE_STEP=t.SCALE_DEFAULT/12,t}();function c(t){var e=document.getElementById(t);if(!e)throw new ReferenceError(t+" is not defined");return e}var h=new URL(window.location.href).searchParams,l=c("sidebar"),d=c("plotview");h.has("sidebar")&&h.get("sidebar")&&(l.classList.add("notransition","nohist"),d.classList.add("notransition","nohist"),setTimeout((function(){l.classList.remove("notransition"),d.classList.remove("notransition")}),300));var u=new r(h.get("host")||window.location.host,h.get("token")||void 0,!h.has("ws")||"0"!=h.get("ws"));window.onload=function(){var t,e=c("tb-more").parentElement;function n(){e.classList.remove("drop-open")}e.onmouseenter=function(){e.classList.add("drop-open")},e.onmouseleave=function(){e.classList.remove("drop-open")},u.onZoomStringChange=function(t){c("tb-zlvl").childNodes[0].nodeValue=t},u.onIndexStringChange=function(t){c("tb-pnum").childNodes[0].nodeValue=t},u.onDisconnectedChange=function(t){c("overlay-text").innerText="Connection lost.",c("overlay").style.display=t?"inline":"none"},u.onDeviceActiveChange=function(e){e?t=setTimeout((function(){c("overlay-text").innerText="Device inactive.",c("overlay").style.display="inline"}),1e3):(clearTimeout(t),c("overlay").style.display="none")},window.addEventListener("resize",(function(){return u.resize()}));var i=c("drawing");u.init(i,c("sidebar"));for(var o={},s=0,a=[{id:"navPrevious",keys:[37,40],fun:function(){return u.navPrevious()},onclickId:"tb-left"},{id:"navNext",keys:[39,38],fun:function(){return u.navNext()},onclickId:"tb-right"},{id:"navNewest",keys:[78],fun:function(){return u.navNewest()},onclickId:"tb-pnum"},{id:"zoomIn",keys:[187],fun:function(){return u.zoomIn()},onclickId:"tb-plus"},{id:"zoomOut",keys:[189],fun:function(){return u.zoomOut()},onclickId:"tb-minus"},{id:"navClear",altKey:!0,keys:[68],fun:function(){u.navClear(),n()},onclickId:"tb-clear"},{id:"navRemove",keys:[46,68],fun:function(){return u.navRemove()},onclickId:"tb-remove"},{id:"downloadPlotSVG",keys:[83],fun:function(){u.downloadPlotSVG(i),n()},onclickId:"tb-save-svg"},{id:"downloadPlotPNG",keys:[80],fun:function(){u.downloadPlotPNG(i),n()},onclickId:"tb-save-png"},{id:"zoomReset",keys:[48],fun:function(){return u.zoomReset()},onclickId:"tb-zlvl"},{id:"copyPlotPNG",keys:[67],fun:function(){u.copyPlotPNG(i),n()},onclickId:"tb-copy-png"},{id:"toggleSidebar",keys:[72],fun:function(){return c("sidebar").classList.toggle("nohist"),c("plotview").classList.toggle("nohist"),setTimeout((function(){return u.checkResize()}),300),void n()},onclickId:"tb-history"},{id:"export",keys:[72],fun:function(){g.style.display="block"},onclickId:"tb-export"}];s<a.length;s++){var r=a[s];if(r.onclickId&&(c(r.onclickId).onclick=r.fun),r.keys)for(var h=0,l=r.keys;h<l.length;h++){var d=l[h];o[d]=r}}window.addEventListener("keydown",(function(t){var e=o[t.keyCode];if(e&&(!e.altKey||t.altKey))return e.fun(),void t.preventDefault()}));var p=c("toolbar"),v=c("container"),f=setTimeout((function(){return p.classList.add("fade-out")}),4e3);v.onmousemove=function(){p.classList.remove("fade-out"),clearTimeout(f),f=setTimeout((function(){return p.classList.add("fade-out")}),4e3)},v.onmouseleave=function(t){p.classList.add("fade-out")};var g=c("exp-modal");document.getElementsByClassName("modal-close")[0].onclick=function(){g.style.display="none"},window.onclick=function(t){t.target==g&&(g.style.display="none")}}})()})();