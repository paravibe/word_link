
-- SUMMARY --

The module allows you to replace certain words on the links. Its very easy to
use. Simply specify what word should be replaced.

Main features:
- Replace word in content with a link.
- Can set on which content type and which field it will be affected.
- Can set the limit of words to be replaced.
- Can set a list of HTML tags that will be ignored.
- Can specify case sensitivity.
- Can set a path on which words will not be replaced.


-- REQUIREMENTS --

PHP 5.


-- INSTALLATION --

* Put the module in your drupal modules directory and enable it in 
  admin/modules. 


-- CONFIGURATION --

* Configure user permissions in Administration » People » Permissions:
  Go to admin/people/permissions and grant permission to any roles
  that need to be able to add and edit words.

* Configure content types and fields:
  You can set node types, fields and replace limit at
  admin/config/content/word-link.

* Add a new word:
  Just follow the admin/config/content/word-link/add 
  link and fill out the form.
