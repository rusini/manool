---
permalink: /blog/
title:     MANOOL Blog
---

<header>
  <h1 id="start">{{page.title|smartify}}</h1>
</header>

News, Random Topics: Examples, Rationale, Hacking...

{%for post in site.posts%}{%unless post.unlisted%}<section>
  <h2>{{post.title|smartify}}</h2>
  <p markdown="1">&hellip;{{post.excerpt}}&hellip;</p>
  <p><a href="{{post.url}}">Complete article</a></p>
  <div class="right"><small>Published: <time datetime="{{post.date|date:'%F'}}">{{post.date|date:'%B %-d, %Y'}}</time></small></div>
  <div class="right"><small>Updated: <time datetime="{{post.updated|date:'%F'}}">{{post.updated|date:'%B %-d, %Y'|default:'never'}}</time></small></div>
</section>{%endunless%}{%endfor%}
