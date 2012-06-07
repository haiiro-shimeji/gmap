---
layout: post
title: "gmap-static"
date: 2012-06-07 18:11
comments: true
categories: 
---

<h2>gmap-static</h2>

<p>
gmap-static is a sub-module of gmap module that alter google map elements to static map images.
<p>

<p>
For mobile devices or smartphones which do not support so high display resolution, it is not suitable to provide full-functional google map. You can define the pages in which it is provided the static map image instead of functional google map by this module.
</p>

<h3>usage</h3>
<p>
When gmap-static module is enabled, you can set the conditions in setting page (admin/config/services/`gmap_static`).

If "All pages except those listed" or "Only the listed pages" is checked, you chan define the page paths in which or not in which the map elements is altered to the static imagei in the following textarea. If "Pages on which this PHP code returns TRUE (experts only)" is checked, you can define the php code when that returns true the alteration is enabled. To use PHP code option, PHP filter module should be enabled.
</p>

<h3>PHP code examples</h3>

This is the example alteration is enabled in the all mobile devices (includes smartphones).

if (`function_exists`("`mobile_tools_is_mobile_device`")) {
	$device = mobile_tools_is_mobile_device();
	if ( 'mobile' == $device['type'] ) )
		return true;
}
return false;


