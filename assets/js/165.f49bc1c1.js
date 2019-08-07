(window.webpackJsonp=window.webpackJsonp||[]).push([[165],{426:function(e,t,s){"use strict";s.r(t);var o=s(38),a=Object(o.a)({},function(){var e=this,t=e.$createElement,s=e._self._c||t;return s("ContentSlotsDistributor",{attrs:{"slot-key":e.$parent.slotKey}},[s("h1",{attrs:{id:"tutorial-lifetime-of-objects-passed-into-wren"}},[s("a",{staticClass:"header-anchor",attrs:{href:"#tutorial-lifetime-of-objects-passed-into-wren","aria-hidden":"true"}},[e._v("#")]),e._v(" Tutorial: Lifetime of objects passed into Wren")]),e._v(" "),s("p",[e._v("In the previous section we have discussed what happens with objects that are returned from Wren, but what about the other way around?")]),e._v(" "),s("p",[e._v("It's very simple in this case. There are only 4 different scenarios:")]),e._v(" "),s("ul",[s("li",[e._v("Pass class "),s("code",[e._v("Foo")]),e._v(" as a value. The Wren will create a copy of it.")]),e._v(" "),s("li",[e._v("Pass class "),s("code",[e._v("Foo*")]),e._v(" as a pointer. The instance will be wrapped into "),s("code",[e._v("std::shared_ptr<Foo>")]),e._v(" "),s("strong",[e._v("but won't free it")]),e._v(". So it will be alive inside of Wren until you free the instance on C++ side. Wren's garbage collector won't free this instance.")]),e._v(" "),s("li",[e._v("Pass class "),s("code",[e._v("Foo&")]),e._v(" as a reference. This is the same case as passing a pointer.")]),e._v(" "),s("li",[e._v("Pass class "),s("code",[e._v("std::shared_ptr<Foo>")]),e._v(" as a shared pointer. The lifetime of this object will be extended by Wren. So both C++ and Wren will manage the lifetime of this. If you lose the shared pointer on C++ side, and Wren holds the instance, then it will be freed by Wren's garbage collector only when the instance inside of Wren gets no longer used/referenced.")])])])},[],!1,null,null,null);t.default=a.exports}}]);