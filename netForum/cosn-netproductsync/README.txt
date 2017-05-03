
ABOUT MODULE
------------

This module is used to synchronize all the product which are present in users netForum account.

There are three action that you can perform after enabling the module which will be - 
 * Save the settings for product sync by clicking on on "Save settings" button
 * Sync the product on very fisrt moment by clicking on on "Sync Now" button
 * Clear all the products from database by clicking on "Clear" button


CONFIGURATION AND INSTALLATION
--------------------------

After enabling the module from back-end (admin panel), every user needs to follow the sync steps first.

STEP -1
 * Go to the configuration and look for NetForum section at the bottom.
 * Click on "netForum Config Settings" to setup the cofiguration towards NetForum.
 * Fill up the all the required and proper data into the fields.
 * Save config.

STEP -2
 * Go to the configuration for module or click on the "netForum Product and Sync".
 * Fill up all the required and proper data into the fields and save settings.
 * After saving the settings you can directly sync all the products by just clicking on the "Sync Now" button.
 * Orelse you can wait for to run cron as the approved time slots.

 STEP -3
 * Go to the configuration -> NetForum -> Product List
 * By clicking on this, you can see the complete list for available/sync products.
 * By clicking on the "Edit" link admin can edit the and check the for the download url for that particular product.


DISPLAYING PRODUCT LIST TO FRONT-END
---------------------

Now the time is for to display the all sync product and its individual product detail page in front-end for the user view. Once the module is installed successfully and enabled properly, it automatically will create the default block for showing the product list and 
product individual detail page.

After doing all above step successfully, we need to show the product view to the user, for that needs to follow the required steps.

 * Go to the Structure -> Block you can see two blocks i.e. "NetForum Product Sync" and "NetForum Product Details" in disabled section.
 * You can put that block in any regions of your site have by just selecting the option.
 * Or else you can create a custom page and display it on programmatically by your own.
 * Or else you can use a panels module to create a page and display it on by selecting the proper module.
