#include "webwireprofile.h"

#include "webwirehandler.h"
#include "execjs.h"

static std::string cssCode(const std::string &css)
{
   return std::string("{") +
        "let styles ='" + css + "';\n"
        "let stylesheet = document.createElement('style');\n"
        "stylesheet.setAttribute('id', 'web-wire-css');\n"
        "stylesheet.textContent = styles;\n"
        "document.head.appendChild(stylesheet);\n"
        "}";
}

#define esc(in) ExecJs::esc_quote(in)

WebWireProfile::WebWireProfile(const std::string &name, const std::string &default_css, Object_t *parent)
    : Object_t(parent)
{
    _profile_name = name;
    _css = default_css;

    Script_t dom_access;

    int world_id = WEB_WIRE_PROFILE_WORLD_ID;

    dom_access.setName("dom_access");
    dom_access.setSourceCode(
        asprintf(
            "window.dom_set_html_%d = function(id, data, do_fetch) {\n"
            "  let el = document.getElementById(id);\n"
            "  if (el !== undefined && el !== null) {\n"
            "     if (do_fetch) { \n"
            "        fetch(data).then(x => x.text()).then(y => el.innerHTML = y);\n"
            "     } else {\n"
            "       el.innerHTML = data;\n"
            "     }\n"
            "     return 'bool:true';\n"
            "  } else {\n"
            "    console.error('element with id ' + id + ' not found');\n"
            "    return 'bool:false';\n"
            "  }\n"
            "};\n"
            "window.dom_get_html_%d = function(id) {\n"
            "  let el = document.getElementById(id);\n"
            "  if (el !== undefined && el !== null) {\n"
            "     return el.innerHTML;\n"
            "  } else {\n"
            "     return '';\n"
            "  }\n"
            "};\n"
            "window.dom_set_attr_%d = function(id, attr, val) {\n"
            "  let el = document.getElementById(id);\n"
            "  if (el === undefined || el === null) {\n"
            "     return 'bool:false';\n"
            "  } else {\n"
            "     el.setAttribute(attr, val);\n"
            "     return 'bool:true';\n"
            "  }\n"
            "};\n"
            "window.dom_get_attr_%d = function(id, attr) {\n"
            "  let el = document.getElementById(id);\n"
            "  if (el === undefined || el === null) {\n"
            "    return '';\n"
            "  } else {\n"
            "    let v = el.getAttribute(attr);\n"
            "    if (v === null) { return ''; }\n"
            "    else { return v; }\n"
            "  }\n"
            "};\n"
            "window.dom_del_attr_%d = function(id, attr) {\n"
            "  let el = document.getElementById(id);\n"
            "  if (el === undefined || el === null) {\n"
            "    return 'bool:false';\n"
            "  } else {\n"
            "    el.removeAttribute(attr);\n"
            "    return 'bool:true';\n"
            "  }\n"
            "};\n"
            "window.split_style_%d = function(st) {\n"
            "  if (st === null) { st = ''; }\n"
            "  let parts = st.split(';');\n"
            "  let n_parts = [];\n"
            "  parts.forEach(function(part) {\n"
            "     if (part != '') {\n"
            "        let kv = part.split(':');\n"
            "        let key = kv[0].trim();\n"
            "        let val = (kv.length == 2) ? kv[1] : '';\n"
            "        n_parts.push({ key: key, val: val});\n"
            "     }\n"
            "  });\n"
            "  return n_parts;\n"
            "};\n"
            "window.dom_add_style_%d = function(id, style) {\n"
            "  let style_parts = split_style_%d(style);\n"
            "  let st = window.dom_get_attr_%d(id, 'style');\n"
            "  if (st === null) { st = ''; }\n"
            "  st_parts = window.split_style_%d(st);\n"
            "  style_parts.forEach(function(style_part) {\n"
            "     let sp_key = style_part.key;\n"
            "     st_parts = st_parts.filter((e) => e.key != sp_key);\n"
            "     st_parts.push(style_part);\n"
            "  });\n"
            "  n_style = '';\n"
            "  st_parts.forEach(function(p) {\n"
            "    n_style += p.key + ':' + p.val + ';';\n"
            "  });\n"
            "  window.dom_set_attr_%d(id, 'style', n_style);\n"
            "};\n"
            "window.dom_set_style_%d = function(id, style) {\n"
            "  window.dom_set_attr_%d(id, 'style', style);\n"
            "};\n"
            "window.dom_get_style_%d = function(id) {\n"
            "  return window.dom_get_attr_%d(id, 'style');\n"
            "};\n"
            "window.dom_set_css_%d = function(css) {\n"
            "  let stylesheet = document.getElementById('web-wire-css');\n"
            "  if (stylesheet) {\n"
            "     stylesheet.textContent = css;\n"
            "     return 'bool:true';\n"
            "  }\n"
            "  return 'bool:false';\n"
            "};\n"
            "window.dom_get_attrs_%d = function(id) {\n"
            "  let el = document.getElementById(id);\n"
            "  if (el === undefined || el === null) {\n"
            "     return 'json:[]';\n"
            "  } else {\n"
            "    let res = [];\n"
            "    let attr_names = el.getAttributeNames();\n"
            "    for(const name of attr_names) {\n"
            "       res.push([name, el.getAttribute(name)]);\n"
            "    }\n"
            "    let ret = 'json:' + JSON.stringify(res);\n"
            "    return ret;\n"
            "  }\n"
            "};\n"
            "window.dom_uid_%d = function() {\n"
            "  return Date.now().toString(36) + Math.random().toString(36).substr(2);\n"
            "};\n"
            "window.dom_get_elements_%d = function(selector) {\n"
            "  let nodelist = document.querySelectorAll(selector);\n"
            "  if (nodelist === undefined || nodelist === null) {\n"
            "     return 'json:[]';\n"
            "  } else {\n"
            "    let els = [];\n"
            "    nodelist.forEach(function(el) { \n"
            "      let id = el.getAttribute('id');\n"
            "      if (id === null) {\n"
            "         id = window.dom_uid_%d();\n"
            "          el.setAttribute('id', id);\n"
            "      }\n"
            "      let attr_names = el.getAttributeNames();\n"
            "      let attrs = [];\n"
            "      for(const name of attr_names) {\n"
            "         attrs.push([name, el.getAttribute(name)]);\n"
            "      }\n"
            "      els.push([el.nodeName, attrs]);\n"
            "    });\n"
            "    return 'json:' + JSON.stringify(els);\n"
            "  }\n"
            "};",
            world_id,   // set_html
            world_id,   // get_html
            world_id,   // set_attr
            world_id,   // get_attr
            world_id,   // del_attr
            world_id,   // split_style
            world_id, world_id, world_id, world_id, world_id,   // add_style
            world_id, world_id,                                 // set_style
            world_id, world_id,                                 // get_style
            world_id,                                            // set-css
            world_id,    // get-attrs
            world_id,    // uid
            world_id, world_id     // get-elements
            )
        );

    Script_t eventing;
    eventing.setName("eventing");
    eventing.setSourceCode(
        std::string() +
        "window._web_wire_evt_queue = [];\n"
        "window._web_wire_queue_worker = function() {\n"
        "  if (typeof web_ui_wire_handle_event === 'function') {\n"
        "     while (window._web_wire_evt_queue.length > 0) {\n"
        "        let evt = window._web_wire_evt_queue.shift();\n"
        "        web_ui_wire_handle_event(JSON.stringify(evt));\n"
        "     }\n"
        "  }\n"
        "  window.setTimeout(window._web_wire_queue_worker, 5);\n"
        "};\n"
        "window.setTimeout(window._web_wire_queue_worker, 15);\n"
        "window._web_wire_put_evt = function(evt) { window._web_wire_evt_queue.push(evt); };\n"
        "window._web_wire_event_info = function(e, id, evt) {\n"
        "  let obj = {};\n"
        "  if (e == 'input') {\n"
        "     obj['data'] = evt.data;\n"
        "     obj['dataTransfer'] = evt.dataTransfer;\n"
        "     obj['inputType'] = evt.inputType;\n"
        "     obj['isComposing'] = evt.isComposing;\n"
        "     obj['value'] = document.getElementById(id).value;\n"
        "  } else if (e == 'change') {\n"
        "     obj['value'] = document.getElementById(id).value;\n"
        "  } else if (e == 'mousemove' || e == 'mouseover' || e == 'mouseenter' || \n"
        "e == 'mouseleave' || e == 'click' || e == 'dblclick' || \n"
        "e == 'mousedown' || e == 'mouseup' ) {\n"
        "     obj['altKey'] = evt.altKey;\n"
        "     obj['buttons'] = evt.buttons;\n"
        "     obj['clientX'] = evt.clientX;\n"
        "     obj['clientY'] = evt.clientY;\n"
        "     obj['ctrlKey'] = evt.ctrlKey;\n"
        "     obj['metaKey'] = evt.metaKey;\n"
        "     obj['movementX'] = evt.movementX;\n"
        "     obj['movementY'] = evt.movementY;\n"
        "     obj['screenX'] = evt.screenX;\n"
        "     obj['screenY'] = evt.screenY;\n"
        "     obj['shiftKey'] = evt.shiftKey;\n"
        "     obj['x'] = evt.x;\n"
        "     obj['y'] = evt.y;\n"
        "  } else if (e == 'keydown' || e == 'keyup' || e == 'keypress') {\n"
        "     obj['key'] = evt.key;\n"
        "     obj['code'] = evt.code;\n"
        "     obj['altKey'] = evt.altKey;\n"
        "     obj['ctrlKey'] = evt.ctrlKey;\n"
        "     obj['metaKey'] = evt.metaKey;\n"
        "     obj['repeat'] = evt.repeat;\n"
        "     obj['shiftKey'] = evt.shiftKey;\n"
        "  }\n"
        // More events can be added like pointerEvent, clipboardEvent, etc.
        "  return obj;\n"
        "};\n"
        "window._web_wire_get_evts = function() {\n"
        "   let v = _web_wire_evt_queue;\n"
        "   _web_wire_evt_queue = [];\n"
        "   return JSON.stringify(v);"      // This needs no extra type info, as it is internally used only
        "};\n"
        "window._web_wire_bind_evt_ids = function(selector, event_kind) {\n"
        "   let nodelist = document.querySelectorAll(selector);\n"
        "   if (nodelist === undefined || nodelist === null) {\n"
        "      return 'json:[]';\n"
        "   }\n"
        "   let ids = [];\n"
        "   nodelist.forEach(function(el) { \n"
        "      let el_id = el.getAttribute('id');\n"
        "      let el_tag = el.nodeName;\n"
        "      let el_type = el.getAttribute('type');\n"
        "      if (el_type === null) { el_type = ''; }\n"
        "      if (el_id !== null) {\n"
        "        el.addEventListener(event_kind, \n"
        "          function(e) {\n"
        "             let obj = {evt: event_kind, id: el_id, js_evt: window._web_wire_event_info(event_kind, el_id, e) };\n"
        "             window._web_wire_put_evt(obj);\n"
        "          }\n"
        "        );\n"
        "        let info = [ el_id, el_tag, el_type ];\n"
        "        ids.push(info);\n"
        "      }\n"
        "   });\n"
        "   return 'json:' + JSON.stringify(ids);\n"
        "};\n"
        "window._web_wire_resize_timeout = false;\n"
        "window.addEventListener('resize', function() {\n"
        "   clearTimeout(window._web_wire_resize_timeout);\n"
        "   let f = function() {\n"
        "     let w = window.outerWidth;\n"
        "     let h = window.outerHeight;\n"
        "     web_ui_wire_resize_event(w, h);\n"
        "   };\n"
        "   window._web_wire_resize_timeout = setTimeout(f, 250);\n"
        "});\n"
        "window._web_wire_x = window.screenX;\n"
        "window._web_wire_y = window.screenY;\n"
        "window._web_wire_move_interval = setInterval(function() {\n"
        "   let x = window.screenX;\n"
        "   let y = window.screenY;\n"
        "   if (x != window._web_wire_x || y != window._web_wire_y) {\n"
        "      window._web_wire_x = x;\n"
        "      window._web_wire_y = y;\n"
        "      web_uit_wire_move_event(x, y);\n"
        "   }\n"
        "}, 500);\n"
     );

    Script_t menus;
    menus.setName("Web Wire Menus");
    menus.setSourceCode(
        "window._web_wire_menu = function(menubar) {\n"
        "	let el = document.getElementById('web-wire-menu');\n"
        "	if (el === null) {\n"
        "		el = document.createElement('div');\n"
        "		el.setAttribute('class', 'menubar');\n"
        "		el.setAttribute('id', 'web-wire-menu');\n"
        "	}\n"
        "	el.innerHTML = '';\n"
        "	let html = '';\n"
        "	let menus = menubar.menu;\n"
        "   if (menus !== undefined) {\n"
        "	   menus.forEach(function (menu) {\n"
        "		   let id = menu.id;\n"
        "		   let name = menu.name.replace(/\\s/g, '&nbsp;');\n"
        "		   html += `<div class=\"menubar-item\" id=\"${id}\">${name}<div id=\"${id}-menu\"></div></div>`;\n"
        "	   });\n"
        "   }\n"
        "	\n"
        "	let openMenu = function(from_id, for_id, submenu) {\n"
        "		let from_el = document.getElementById(from_id);\n"
        "		let el = document.getElementById(for_id);\n"
        "		let html = '<div class=\"menu\">';\n"
        "		let clear_menu = function(the_id) {\n"
        "			setTimeout(function() {\n"
        "				let e = document.getElementById(the_id);\n"
        "				if (e) { e.innerHTML = '';  }\n"
        "			}, 50);\n"
        "		};\n"
        "		submenu.menu.forEach(function (menu_item) {\n"
        "			let sep = menu_item.separator;\n"
        "			let add_cl = '';\n"
        "			if (sep) {\n"
        "				add_cl = ' separator';\n"
        "			} \n"
        "			let id = menu_item.id;\n"
        "			let name = menu_item.name.replace(/\\s/g, '&nbsp;');\n"
        "			let icon = menu_item.icon;\n"
        "			if (icon !== undefined) {\n"
        "				icon = '<img src=\"' + icon + '\" />';\n"
        "			} else {\n"
        "				icon = '';\n"
        "			}\n"
        "			let submenu = menu_item.menu;\n"
        "			let indicator = (submenu === undefined) ? '' : '&gt;';\n"
        "			let menu_el = (submenu === undefined) ? '' : '<div class=\"submenu\" id=\"' + id + '-menu\"></div>';\n"
        "			html += `<div class=\"menu-item ${add_cl}\" id=\"${id}\"><span class=\"menu-icon\">${icon}</span><span class=\"menu-name\">${name}</span><span class=\"menu-submenu\">${indicator}</span>${menu_el}</div>`;\n"
        "		});\n"
        "		html += '</div>';\n"
        "		setTimeout(function () {\n"
        "			document.addEventListener('click', function(e) {\n"
        "				if (!el.contains(e.target) && !from_el.contains(e.target)) {\n"
        "					clear_menu(for_id);\n"
        "				}\n"
        "			}, { once: true });\n"
        "			},\n"
        "			0);\n"
        "		el.innerHTML = html;\n"
        "		let menu_el = el;\n"
        "		submenu.menu.forEach(function (menu_item) {\n"
        "			let id = menu_item.id;\n"
        "			let el = document.getElementById(id);\n"
        "			let submenu = menu_item.menu;\n"
        "			if (el !== null) {\n"
        "				if (submenu !== undefined) {\n"
        "				   el.addEventListener('mouseenter', function() {\n"
        "						let mnu = document.getElementById(id + '-menu');\n"
        "						let open = mnu.getAttribute('open');\n"
        "						if (open === null) {\n"
        "							mnu.setAttribute('open', 'yes');\n"
        "							openMenu(id, id + '-menu', submenu);\n"
        "						}\n"
        "				   });\n"
        "				   el.addEventListener('mouseleave', function(evt) {\n"
        "						let mnu = document.getElementById(id + '-menu');\n"
        "						if (!mnu.contains(evt.target)) {\n"
        "							clear_menu(id + '-menu');\n"
        "							mnu.removeAttribute('open');\n"
        "						}\n"
        "				   });\n"
        "				} else {\n"
        "				   el.addEventListener('click', function() {\n"
        "						let obj = { evt: 'menu-item-choosen', item: id };\n"
        "						window._web_wire_put_evt(obj);\n"
        "						clear_menu(for_id);\n"
        "					});\n"
        "				   //window._web_wire_bind_evt_ids(`#${id}`, 'click');\n"
        "				}\n"
        "			}\n"
        "		});\n"
        "	};\n"
        "	document.body.prepend(el);\n"
        "	el.innerHTML = html;\n"
        "   if (menus !== undefined) {\n"
        "	  menus.forEach(function (menu) {\n"
        "		let id = menu.id;\n"
        "		let submenu = menu.submenu;\n"
        "		let el = document.getElementById(id);\n"
        "		if (el !== null) {\n"
        "			if (submenu) {\n"
        "				el.addEventListener('click', function() {\n"
        "					openMenu(id, id + '-menu', submenu);\n"
        "				});\n"
        "			} else {\n"
        "				window._web_wire_bind_evt_ids(`#${id}`, 'click');\n"
        "			}\n"
        "		}\n"
        "	  });\n"
        "   }\n"
        "};\n"
     );

    Script_t onload;
    onload.setName("onload-event");
    onload.setSourceCode("window.addEventListener('load', function () {"
                         "  let obj = { evt: 'page-loaded', page_handle: window._page_handle };"
                         "  window._web_wire_put_evt(obj);"
                         "}, { once: true });"
                         );

    _css_script.setName("css");
    _css_script.setSourceCode(cssCode(esc(_css)));

    _scripts.clear();
    _scripts.push_back(dom_access);
    _scripts.push_back(eventing);
    _scripts.push_back(_css_script);
    _scripts.push_back(menus);
    _scripts.push_back(onload);

    _set_html_name = asprintf("window.dom_set_html_%d", world_id);
    _get_html_name = asprintf("window.dom_get_html_%d", world_id);
    _set_attr_name = asprintf("window.dom_set_attr_%d", world_id);
    _get_attr_name = asprintf("window.dom_get_attr_%d", world_id);
    _get_attrs_name = asprintf("window.dom_get_attrs_%d", world_id);
    _del_attr_name = asprintf("window.dom_del_attr_%d", world_id);
    _add_style_name = asprintf("window.dom_add_style_%d", world_id);
    _set_style_name = asprintf("window.dom_set_style_%d", world_id);
    _get_style_name = asprintf("window.dom_get_style_%d", world_id);
    _get_elements_name = asprintf("window.dom_get_elements_%d", world_id);
    _set_css_name = asprintf("window.dom_set_css_%d", world_id);

    //_world_id = dom_access.worldId();

    //fprintf(stderr, "world-id of domaccess: %d\n", dom_access.worldId());
    //fprintf(stderr, "world-id of eventing: %d\n", eventing.worldId());fflush(stderr);

    int usage = 0;
    setProperty("__win_wire_usage_count__", usage);
}

void WebWireProfile::decUsage()
{
    int usage = property("__win_wire_usage_count__").toInt();
    usage -= 1;
    if (usage > 0) {
        setProperty("__win_wire_usage_count__", usage);
    } else {
        this->deleteLater();    // The profile is now not used anymore,
                                // but we want to be sure it is not somewhere in use anymore
                                // So, we delete it later (in the eventloop).
    }
}

void WebWireProfile::incUsage()
{
    int usage = property("__win_wire_usage_count__").toInt();
    usage += 1;
    setProperty("__win_wire_usage_count__", usage);
}

int WebWireProfile::usage()
{
    int usage = property("__win_wire_usage_count__").toInt();
    return usage;
}

std::string WebWireProfile::profileName()
{
    return _profile_name;
}

void WebWireProfile::exec(WebWireHandler *h, int win, const std::string &name, const std::string &js)
{
    h->message(js);
    ExecJs e = ExecJs(h, win, name, true);
    e.run(js);
}

void WebWireProfile::exec(WebWireHandler *h, int win, const std::string &name, const std::string &js, bool &ok, std::string &result)
{
    h->message(js);
    ExecJs e(h, win, name, false);
    result = e.call(js, ok);
}


void WebWireProfile::set_html(WebWireHandler *h, int win, const std::string &element_id, const std::string &data, bool fetch)
{
    std::string do_fetch = fetch ? "true" : "false";

    exec(h, win, "set-inner-html",
                _set_html_name + "(" + "'" + esc(element_id) + "', " +
                                          "'" + esc(data) + "', " +
                                          do_fetch +
                                    ");"
              );
}

std::string WebWireProfile::get_html(WebWireHandler *h, int win, const std::string &element_id, bool &ok)
{
    std::string result;
    exec(h, win, "get-inner-html",
                "return " + _get_html_name + "('" + esc(element_id) + "');",
                ok, result
                );
    return result;
}

void WebWireProfile::set_attr(WebWireHandler *h, int win, const std::string &element_id, const std::string &attr, const std::string &val)
{
    exec(h, win, "set-attr",
                _set_attr_name + "(" + "'" + esc(element_id) + "', " +
                                          "'" + esc(attr) + "', " +
                                          "'" + esc(val) + "'" +
                                    ");"
              );
}

std::string WebWireProfile::get_attr(WebWireHandler *h, int win, const std::string &element_id, const std::string &attr, bool &ok)
{
    std::string result;
    exec(h, win, "get-attr",
                "return " + _get_attr_name + "(" +  "'" + esc(element_id) + "', " +
                                           "'" + esc(attr) + "'" +
                                    ");",
                ok, result
              );
    return result;
}

std::string WebWireProfile::get_attrs(WebWireHandler *h, int win, const std::string &element_id, bool &ok)
{
    std::string result;
    exec(h, win, "get-attrs",
                "return " + _get_attrs_name + "(" +  "'" + esc(element_id) + "'" +
                    ");",
                ok, result
                );
    return result;
}

std::string WebWireProfile::get_elements(WebWireHandler *h, int win, const std::string &selector, bool &ok)
{
    std::string result;
    exec(h, win, "get-elements",
                "return " + _get_elements_name + "(" +  "'" + esc(selector) + "'" +
                    ");",
                ok, result
                );
    return result;
}


void WebWireProfile::del_attr(WebWireHandler *h, int win, const std::string &element_id, const std::string &attr)
{
    exec(h, win, "del-attr",
                _del_attr_name + "(" +  "'" + esc(element_id) + "', " +
                              "'" + esc(attr) + "', " +
                              ");"
                     );
}

void WebWireProfile::add_style(WebWireHandler *h, int win, const std::string &element_id, const std::string &style)
{
    exec(h, win, "add-style",
                _add_style_name + "(" +  "'" + esc(element_id) + "', " +
                    "'" + esc(style) + "'" +
                    ");"
                );
}


void WebWireProfile::set_style(WebWireHandler *h, int win, const std::string &element_id, const std::string &style)
{
    return exec(h, win, "set-style",
                _set_style_name + "(" +  "'" + esc(element_id) + "', " +
                                            "'" + esc(style) + "'" +
                                     ");"
              );
}

std::string WebWireProfile::get_style(WebWireHandler *h, int win, const std::string &element_id, bool &ok)
{
    std::string result;
    exec(h, win, "get-style",
                "return " + _get_style_name + "('" + esc(element_id) + "');",
                ok, result
                );
    return result;
}

void WebWireProfile::set_css(WebWireHandler *h, int win, const std::string &css)
{
    _css  = css;
    _css_script.setSourceCode(cssCode(esc(css)));

    exec(h, win, "set-css",
                _set_css_name + "('" + esc(css) + "');"
                );
}

std::string WebWireProfile::scriptsTag()
{
    std::list<Script_t>::iterator it = _scripts.begin();
    std::string tag = "";
    while (it != _scripts.end()) {
        tag += "<script>";
        Script_t &s = *it;
        tag += "\n// " + s.name() + "\n" + s.code() + "\n";
        tag += "</script>";
        it++;
    }
    return tag;
}







